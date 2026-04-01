#include "Solver.h"
#include "LigandCSVLoader.h"
#include "Constants.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>

// Constructor
CationSystem::CationSystem() {
    // Default values
}

// Destructor
CationSystem::~CationSystem() {
    Clear();
}

// Initialize with specific ligands and metals
void CationSystem::Initialize(const std::vector<std::string>& ligandNames,
                              const std::vector<std::string>& metalNames) {
    // Find and add ligands
    for (const auto& name : ligandNames) {
        const Ligand* ligand = GetLigandByName(name);
        if (ligand) {
            ligands.push_back(*ligand);
        }
    }

    // Find and add metals
    for (const auto& name : metalNames) {
        const Metal* metal = GetMetalByName(name);
        if (metal) {
            metals.push_back(*metal);
        }
    }
}

// Marshall-Franck equation for pKw(T, I)
double CationSystem::calculatePKw(double temperature, double ionicStrength) {
    double T_kelvin = temperature + 273.15;
    
    // pKw at I=0 (thermodynamic)
    // pKw = -14.9435 + 3223.0/T + 0.016310*T
    // This is a simplified Marshall-Franck variant for biological range
    double pKw_zero = 14.0; // Default at 25C
    if (std::abs(temperature - 25.0) > 0.1) {
        pKw_zero = 4470.99 / T_kelvin - 6.0875 + 0.01706 * T_kelvin;
    }

    if (ionicStrength <= 0) return pKw_zero;

    // Ionic strength correction for pKw (using Davies for H+ and OH- activity)
    double I_molar = ionicStrength / 1000.0;
    double e = 87.7251 - 0.3974762 * temperature + 8.253E-4 * temperature * temperature;
    double A = 1.8246E6 / std::pow(e * T_kelvin, 1.5);
    
    // log10(Kw) = log10(Kw_zero) - log10(gamma_H * gamma_OH)
    // gamma_H and gamma_OH both z=1
    double logGamma = -A * 1.0 * (std::sqrt(I_molar) / (1.0 + std::sqrt(I_molar)) - 0.3 * I_molar);
    
    return pKw_zero + 2.0 * logGamma;
}

double CationSystem::calculateHydroxideConcentration(double pH, double temperature, double ionicStrength) {
    double pKw = calculatePKw(temperature, ionicStrength);
    
    // Industry Standard: pH is -log10(activity_H)
    // log10(conc_OH) = log10(Kw) - log10(activity_H) - log10(gamma_OH)
    // log10(conc_OH) = -pKw + pH - log10(gamma_OH)
    
    double I_molar = ionicStrength / 1000.0;
    double e = 87.7251 - 0.3974762 * temperature + 8.253E-4 * temperature * temperature;
    double A = 1.8246E6 / std::pow(e * (temperature + 273.15), 1.5);
    double logGamma_OH = -A * 1.0 * (std::sqrt(I_molar) / (1.0 + std::sqrt(I_molar)) - 0.3 * I_molar);
    
    double logOH = -pKw + pH - logGamma_OH;
    return std::pow(10.0, logOH);
}

// Set system parameters
void CationSystem::SetParameters(const SystemParameters& systemParams) {
    params = systemParams;
}

// Activity correction for hydrogen ion concentration based on WebMaxC/NIST methodology
// Returns TH factor where [H+] = 10^-pH / TH
double CationSystem::calculateHActivityCorrection(double /*pH*/, double temperature, double ionicStrength) {
    if (ionicStrength <= 0) return 1.0;

    double I_molar = ionicStrength / 1000.0;

    if (params.mode == SystemParameters::SolverMode::IndustryStandard) {
        // Industry Standard: Use Davies Equation for H+ (z=1) activity coefficient
        double e = 87.7251 - 0.3974762 * temperature + 8.253E-4 * temperature * temperature;
        double A = 1.8246E6 / std::pow(e * (temperature + 273.15), 1.5);
        double logGamma = -A * 1.0 * 1.0 * (std::sqrt(I_molar) / (1.0 + std::sqrt(I_molar)) - 0.3 * I_molar);
        return std::pow(10.0, logGamma);
    }

    // WebMaxC Legacy Logic
    double B = 0.522932 * std::exp(0.0327016 * temperature) + 4.015942;
    double TH = 0.145045 * std::exp(-B * I_molar) + 0.063546 * std::exp(-43.97704 * I_molar) + 0.695634;

    return TH;
}

// Calculate protonation fraction (alpha) for fully deprotonated form (Ln-)
double CationSystem::calculateProtonationFraction(double pH, const Ligand* ligand) {
    if (!ligand) return 1.0;
    
    double TH = calculateHActivityCorrection(pH, params.temperature, params.ionicStrength);
    double H = std::pow(10, -pH) / TH;
    
    double pK1 = ligand->constants.log_K1;
    double pK2 = ligand->constants.log_K2;
    double pK3 = ligand->constants.log_K3;
    double pK4 = ligand->constants.log_K4;
    
    double Tcon1 = std::pow(10, pK1);
    double Tcon2 = std::pow(10, pK2);
    double Tcon3 = std::pow(10, pK3);
    double Tcon4 = std::pow(10, pK4);
    
    // ZSumL = 1 + K1*H + K1*K2*H^2 + K1*K2*K3*H^3 + K1*K2*K3*K4*H^4
    double sum = 1.0;
    double term = 1.0;
    if (pK1 > 0) { term *= Tcon1 * H; sum += term; }
    if (pK1 > 0 && pK2 > 0) { term *= Tcon2 * H; sum += term; }
    if (pK1 > 0 && pK2 > 0 && pK3 > 0) { term *= Tcon3 * H; sum += term; }
    if (pK1 > 0 && pK2 > 0 && pK3 > 0 && pK4 > 0) { term *= Tcon4 * H; sum += term; }
    
    return 1.0 / sum;
}

double CationSystem::calculateProtonationFraction(double pH, double logK) {
    return 1.0 / (1.0 + std::pow(10, logK - pH));
}

double CationSystem::calculateIonicStrengthCorrection(double ionicStrength, double charge) {
    if (ionicStrength <= 0) return 1.0;
    double I_molar = ionicStrength / 1000.0;
    const double A = 0.51;
    double sqrtI = std::sqrt(I_molar);
    double logGamma = -A * charge * charge * (sqrtI / (1.0 + sqrtI) - 0.3 * I_molar);
    return std::pow(10, logGamma);
}

double CationSystem::calculateTemperatureCorrection(double temperature, double deltaH) {
    const double R = 8.314462618;
    const double T1 = 298.15;
    double T2 = temperature + 273.15;
    if (deltaH == 0.0 || T2 == T1) return 1.0;
    
    // Convert kcal/mol to J/mol
    double deltaH_J = deltaH * 4184.0;
    
    // Standard Van 't Hoff: ln(K2/K1) = -(dH/R) * (1/T2 - 1/T1)
    double lnK_ratio = -(deltaH_J / R) * (1.0 / T2 - 1.0 / T1);
    return std::exp(lnK_ratio);
}

double CationSystem::calculateStabilityConstant(double logK, double /*pH*/, double ionicStrength) {
    if (logK <= 0.0) return 0.0;
    double K = std::pow(10.0, logK);
    double gamma_Ca = calculateIonicStrengthCorrection(ionicStrength, 2.0);
    double gamma_L = calculateIonicStrengthCorrection(ionicStrength, -4.0);
    double gamma_complex = calculateIonicStrengthCorrection(ionicStrength, -2.0);
    double adjustedK = K * (gamma_Ca * gamma_L / gamma_complex);
    return adjustedK > 0.0 ? std::log10(adjustedK) : 0.0;
}

double CationSystem::calculateStabilityConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    
    double t = params.temperature;
    double i = params.ionicStrength / 1000.0; 
    double Et = 25.0; // reference temp
    double Ei = 0.1;  // reference ionic strength
    
    // WebMaxC logic for constant adjustment
    auto get_L_factor = [](double temp, double ionic) {
        if (ionic <= 0) return 0.0;
        // WebMaxC: e = 87.7251 - 0.3974762*t + 8.253E-4*t*t
        double e = 87.7251 - 0.3974762 * temp + 8.253E-4 * temp * temp;
        // WebMaxC: Ap = 1.8246E6 / Math.exp(1.5*Math.log(e*(t+273.16)))
        double Ap = 1.8246E6 / std::pow(e * (temp + 273.16), 1.5);
        // WebMaxC: Lfp = Ap*((Math.sqrt(i)/(1+Math.sqrt(i))) - 0.25*i)
        return Ap * ((std::sqrt(ionic) / (1.0 + std::sqrt(ionic))) - 0.25 * ionic);
    };
    
    double Lf = get_L_factor(Et, Ei);
    double Lfp = get_L_factor(t, i);
    double dT_term = (1.0 / (Et + 273.0)) - (1.0 / (t + 273.0));
    const double DH_CONST = 0.00457652;
    const double LN10 = 2.30258509299404568402; // std::log(10.0)

    auto adjust_K = [&](double logK, double dH, int vL, int vM, bool is_MHL = false) {
        if (logK == 0.0) return 0.0;

        if (params.mode == SystemParameters::SolverMode::IndustryStandard) {
            int z_L = -vL; 
            if (is_MHL) z_L = -(vL - 1);
            int z_M = vM;
            int z_complex = z_M + z_L;
            double delta_z2 = (z_complex * z_complex) - (z_M * z_M) - (z_L * z_L);
            
            double e = 87.7251 - 0.3974762 * t + 8.253E-4 * t * t;
            double A = 1.8246E6 / std::pow(e * (t + 273.15), 1.5);
            auto f_I = [](double I) { return (std::sqrt(I) / (1.0 + std::sqrt(I))) - 0.3 * I; };
            double logGamma_correction = -A * delta_z2 * (f_I(i) - f_I(Ei));
            
            double dT_term_van = (1.0 / (Et + 273.15)) - (1.0 / (t + 273.15));
            const double R = 8.314462618;
            double dH_J = dH * 4184.0;
            double logK_T_correction = -(dH_J / (R * LN10)) * dT_term_van;
            
            double K_final = logK + logGamma_correction + logK_T_correction;
            return std::exp(K_final * LN10);
        }

        double K_ionic = logK + (2.0 * vL * vM) * (Lf - Lfp);
        double K_final = K_ionic + (dH * dT_term) / DH_CONST;
        if (K_final < 0) K_final = 0;
        return std::exp(K_final * LN10);
    };

    double TH = calculateHActivityCorrection(params.pH, t, params.ionicStrength);
    double H = (std::exp((-params.pH) * LN10)) / TH;
    
    int vL = ligand->valence;
    if (params.mode == SystemParameters::SolverMode::WebMaxC) {
        vL = 0;
        if (ligand->constants.log_K1 > 0) vL++;
        if (ligand->constants.log_K2 > 0) vL++;
        if (ligand->constants.log_K3 > 0) vL++;
        if (ligand->constants.log_K4 > 0) vL++;
    }

    double K_H1 = adjust_K(ligand->constants.log_K1, ligand->constants.dlog_K1, vL, 1, false);
    double K_H2 = adjust_K(ligand->constants.log_K2, ligand->constants.dlog_K2, vL-1, 1, false);
    double K_H3 = adjust_K(ligand->constants.log_K3, ligand->constants.dlog_K3, vL-2, 1, false);
    double K_H4 = adjust_K(ligand->constants.log_K4, ligand->constants.dlog_K4, vL-3, 1, false);
    
    // WebMaxC makekon logic:
    double t1 = K_H1 * H;
    double t2 = K_H2 * t1 * H;
    double t3 = K_H3 * t2 * H;
    double t4 = K_H4 * t3 * H;
    double ZSumL = 1.0 + t1 + t2 + t3 + t4;
    
    double ZSumL2 = 0;
    if (K_H1 > 0) {
        ZSumL2 = 1.0/(K_H1*H) + 1.0 + K_H2*H + K_H2*K_H3*H*H + K_H2*K_H3*K_H4*H*H*H;
    }

    int vM = GetMetalProperties(metalName).charge;
    if (vM == 0) vM = 2; // Default to divalent if not found
    
    double KML_raw = adjust_K(getMetalBindingConstant(ligand, metalName), 
                         getMetalEnthalpyConstant(ligand, metalName), vL, vM, false);
    
    double KHML_raw = adjust_K(getMetalHBindingConstant(ligand, metalName), 
                          getMetalHEnthalpyConstant(ligand, metalName), vL - 1, vM, true);
    
    double KML_eff = (ZSumL > 0) ? KML_raw / ZSumL : 0;
    double KHML_eff = (ZSumL2 > 0) ? KHML_raw / ZSumL2 : 0;
    
    double effectiveK = KML_eff + KHML_eff;
    
    return effectiveK > 0.0 ? std::log10(effectiveK) : 0.0;
}

double CationSystem::getMetalBindingConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    if (metalName == "Ca2") return ligand->constants.Ca1;
    if (metalName == "Mg2") return ligand->constants.Mg1;
    if (metalName == "Ba2") return ligand->constants.Ba1;
    if (metalName == "Cd2") return ligand->constants.Cd1;
    if (metalName == "Sr2") return ligand->constants.Sr1;
    if (metalName == "Mn2") return ligand->constants.Mn1;
    if (metalName == "X1")  return ligand->constants.X1;
    if (metalName == "Cu2") return ligand->constants.Cu1;
    if (metalName == "Zn2") return ligand->constants.Zn1;
    return 0.0;
}

double CationSystem::getMetalHBindingConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    if (metalName == "Ca2") return ligand->constants.CaH1;
    if (metalName == "Mg2") return ligand->constants.MgH1;
    if (metalName == "Ba2") return ligand->constants.BaH1;
    if (metalName == "Cd2") return ligand->constants.CdH1;
    if (metalName == "Sr2") return ligand->constants.SrH1;
    if (metalName == "Mn2") return ligand->constants.MnH1;
    if (metalName == "X1")  return ligand->constants.XH1;
    if (metalName == "Cu2") return ligand->constants.CuH1;
    if (metalName == "Zn2") return ligand->constants.ZnH1;
    return 0.0;
}

double CationSystem::getMetalEnthalpyConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    if (metalName == "Ca2") return ligand->constants.dCa1;
    if (metalName == "Mg2") return ligand->constants.dMg1;
    if (metalName == "Ba2") return ligand->constants.dBa1;
    if (metalName == "Cd2") return ligand->constants.dCd1;
    if (metalName == "Sr2") return ligand->constants.dSr1;
    if (metalName == "Mn2") return ligand->constants.dMn1;
    if (metalName == "X1")  return ligand->constants.dX1;
    if (metalName == "Cu2") return ligand->constants.dCu1;
    if (metalName == "Zn2") return ligand->constants.dZn1;
    return 0.0;
}

double CationSystem::getMetalHEnthalpyConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    if (metalName == "Ca2") return ligand->constants.dCaH1;
    if (metalName == "Mg2") return ligand->constants.dMgH1;
    if (metalName == "Ba2") return ligand->constants.dBaH1;
    if (metalName == "Cd2") return ligand->constants.dCdH1;
    if (metalName == "Sr2") return ligand->constants.dSrH1;
    if (metalName == "Mn2") return ligand->constants.dMnH1;
    if (metalName == "X1")  return ligand->constants.dXH1;
    if (metalName == "Cu2") return ligand->constants.dCuH1;
    if (metalName == "Zn2") return ligand->constants.dZnH1;
    return 0.0;
}

double CationSystem::solveForFreeMetal(double totalMetal, double totalLigand, double K, double tolerance, int maxIterations) {
    if (K <= 0 || totalMetal <= 0 || totalLigand <= 0) return 0.0;
    double low = 0, high = std::min(totalMetal, totalLigand);
    if (high < tolerance) return 0.0;
    for (int i = 0; i < maxIterations; ++i) {
        double complex = (low + high) / 2.0;
        double predictedComplex = K * (totalMetal - complex) * (totalLigand - complex);
        if (predictedComplex > complex) low = complex; else high = complex;
        if (high - low < tolerance) break;
    }
    return (low + high) / 2.0;
}

CationSystem::MultiSpeciesResult CationSystem::CalculateSimultaneousEquilibrium(
    const std::vector<std::string>& ligandNames, 
    const std::vector<double>& totalLigands,
    const std::vector<std::string>& metalNames, 
    const std::vector<double>& metalInputs,
    const std::vector<bool>& isInputFree) {

    MultiSpeciesResult result;
    result.ligandNames = ligandNames; result.totalLigands = totalLigands;
    result.metalNames = metalNames; 
    result.converged = false;
    size_t m = ligandNames.size(), n = metalNames.size();
    if (m == 0 || n == 0) return result;

    result.freeMetals.resize(n);
    result.totalMetals.resize(n);

    std::vector<double> freemetal(n);
    std::vector<double> freechelator(m);
    std::vector<std::vector<double>> ZMC(n, std::vector<double>(m, 0.0));
    std::vector<double> ZSumL(m, 1.0);
    std::vector<double> Tcon1(m, 0.0);

    const double LN10 = 2.30258509299404568402;
    const double DH_CONST = 0.00457652;
    double Et = 25.0; double Ei = 0.1;
    double t = params.temperature;
    double ionic = params.ionicStrength / 1000.0;

    auto get_L = [](double temp, double ion) {
        if (ion <= 0) return 0.0;
        double e = 87.7251 - 0.3974762 * temp + 8.253E-4 * temp * temp;
        double Ap = 1.8246E6 / std::pow(e * (temp + 273.16), 1.5);
        return Ap * ((std::sqrt(ion) / (1.0 + std::sqrt(ion))) - 0.25 * ion);
    };
    double Lf_corr = get_L(Et, Ei);
    double Lfp_corr = get_L(t, ionic);
    double dT = (1.0 / (Et + 273.0)) - (1.0 / (t + 273.0));

    auto adj = [&](double logK, double dH, int vL, int vM, bool is_MHL = false) {
        if (logK == 0.0) return 0.0;
        if (params.mode == SystemParameters::SolverMode::IndustryStandard) {
            int z_L = -vL; if (is_MHL) z_L = -(vL - 1);
            int z_complex = vM + z_L;
            double delta_z2 = (z_complex * z_complex) - (vM * vM) - (z_L * z_L);
            double e = 87.7251 - 0.3974762 * t + 8.253E-4 * t * t;
            double A = 1.8246E6 / std::pow(e * (t + 273.15), 1.5);
            auto f_I = [](double I) { return (std::sqrt(I) / (1.0 + std::sqrt(I))) - 0.3 * I; };
            double logGamma_correction = -A * delta_z2 * (f_I(ionic) - f_I(Ei));
            double dT_term_van = (1.0 / (Et + 273.15)) - (1.0 / (t + 273.15));
            const double R_val = 8.314462618;
            double logK_T = -(dH * 4184.0 / (R_val * LN10)) * dT_term_van;
            return std::exp((logK + logGamma_correction + logK_T) * LN10);
        }
        double Kf = logK + (2.0 * vL * vM) * (Lf_corr - Lfp_corr) + (dH * dT) / DH_CONST;
        return (Kf > 0) ? std::exp(Kf * LN10) : 0.0;
    };

    double TH = calculateHActivityCorrection(params.pH, t, params.ionicStrength);
    double H = std::pow(10.0, -params.pH) / TH;

    for (size_t i = 0; i < m; ++i) {
        const Ligand* l = GetLigandByName(ligandNames[i]);
        if (!l) continue;
        int vL = l->valence;
        if (params.mode == SystemParameters::SolverMode::WebMaxC) {
            vL = 0;
            if (l->constants.log_K1 > 0) vL++;
            if (l->constants.log_K2 > 0) vL++;
            if (l->constants.log_K3 > 0) vL++;
            if (l->constants.log_K4 > 0) vL++;
        }
        double KH1 = adj(l->constants.log_K1, l->constants.dlog_K1, vL, 1, false);
        double KH2 = adj(l->constants.log_K2, l->constants.dlog_K2, vL-1, 1, false);
        double KH3 = adj(l->constants.log_K3, l->constants.dlog_K3, vL-2, 1, false);
        double KH4 = adj(l->constants.log_K4, l->constants.dlog_K4, vL-3, 1, false);
        Tcon1[i] = KH1 * H;
        ZSumL[i] = 1.0 + Tcon1[i] + KH2*Tcon1[i]*H + KH3*KH2*Tcon1[i]*H*H + KH4*KH3*KH2*Tcon1[i]*H*H*H;
        for (size_t j = 0; j < n; ++j) {
            int vM = GetMetalProperties(metalNames[j]).charge; if (vM == 0) vM = 2;
            double KML = adj(getMetalBindingConstant(l, metalNames[j]), getMetalEnthalpyConstant(l, metalNames[j]), vL, vM, false);
            double KHML = adj(getMetalHBindingConstant(l, metalNames[j]), getMetalHEnthalpyConstant(l, metalNames[j]), vL-1, vM, true);
            ZMC[j][i] = KML + (KHML * Tcon1[i]);
        }
    }

    double freeOH = 0.0;
    std::vector<double> moh_adj(n, 0.0);
    if (params.mode == SystemParameters::SolverMode::IndustryStandard) {
        freeOH = calculateHydroxideConcentration(params.pH, params.temperature, params.ionicStrength);
        for (size_t j = 0; j < n; ++j) {
            const HydroxideConstants* hc = GetHydroxideConstants(metalNames[j]);
            if (hc) {
                int z_M = GetMetalProperties(metalNames[j]).charge;
                double delta_z2 = (double)((z_M-1)*(z_M-1)) - (double)(z_M*z_M) - 1.0;
                double e_val = 87.7251 - 0.3974762 * t + 8.253E-4 * t * t;
                double Ap_val = 1.8246E6 / std::pow(e_val * (t + 273.15), 1.5);
                auto f_I = [](double I) { return (std::sqrt(I) / (1.0 + std::sqrt(I))) - 0.3 * I; };
                double logGamma_correction = -Ap_val * delta_z2 * f_I(ionic);
                double logK_T = -(hc->dH_MOH * 4184.0 / (8.314462618 * LN10)) * ((1.0 / 298.15) - (1.0 / (t + 273.15)));
                moh_adj[j] = std::exp((hc->logK_MOH + logGamma_correction + logK_T) * LN10);
            }
        }
    }

    // Initial guesses
    for (size_t j = 0; j < n; ++j) {
        if (isInputFree[j]) freemetal[j] = metalInputs[j];
        else freemetal[j] = metalInputs[j] / 2.0;
    }
    for (size_t i = 0; i < m; ++i) freechelator[i] = totalLigands[i] / ZSumL[i];

    for (int w = 0; w < 1000; w++) {
        bool converged = true;
        for (size_t i = 0; i < m; ++i) {
            double sum_ZMC_Mf = 0;
            for (size_t j = 0; j < n; ++j) sum_ZMC_Mf += (ZMC[j][i] * freemetal[j]);
            double n_val = totalLigands[i] / (ZSumL[i] + sum_ZMC_Mf);
            double diff = std::abs(n_val - freechelator[i]);
            if (diff > 1e-18 && diff > 0.0001 * freechelator[i]) converged = false;
            freechelator[i] = n_val;
        }
        for (size_t j = 0; j < n; ++j) {
            if (isInputFree[j]) continue; // Target Free metals are constant
            double sum_ZMC_Lf = 0;
            for (size_t i = 0; i < m; ++i) sum_ZMC_Lf += (ZMC[j][i] * freechelator[i]);
            double n_val = metalInputs[j] / (1.0 + sum_ZMC_Lf + moh_adj[j] * freeOH);
            double diff = std::abs(n_val - freemetal[j]);
            if (diff > 1e-18 && diff > 0.0001 * freemetal[j]) converged = false;
            freemetal[j] = n_val;
        }
        if (converged) { result.converged = true; break; }
    }

    result.freeMetals = freemetal;
    result.freeLigands = freechelator;
    result.complexes.assign(m, std::vector<double>(n));
    for (size_t j = 0; j < n; ++j) {
        double sum_ZMC_Lf = 0;
        for (size_t i = 0; i < m; ++i) {
            double c = ZMC[j][i] * freechelator[i] * freemetal[j];
            result.complexes[i][j] = c;
            sum_ZMC_Lf += (ZMC[j][i] * freechelator[i]);
        }
        if (isInputFree[j]) {
            result.totalMetals[j] = freemetal[j] * (1.0 + sum_ZMC_Lf + moh_adj[j] * freeOH);
        } else {
            result.totalMetals[j] = metalInputs[j];
        }
    }
    return result;
}

CationSystem::MultiSpeciesResult CationSystem::CalculateSimultaneousEquilibrium(
    const std::vector<std::string>& ligandNames, const std::vector<double>& totalLigands,
    const std::vector<std::string>& metalNames, const std::vector<double>& totalMetals) {
    std::vector<bool> isFree(metalNames.size(), false);
    return CalculateSimultaneousEquilibrium(ligandNames, totalLigands, metalNames, totalMetals, isFree);
}

// Wrapper for legacy tests
EquilibriumMultiResult CationSystem::CalculateTotalToFreeMulti(double totalLigand, const std::vector<double>& totalMetals,
                                                    const std::string& ligandName, const std::vector<std::string>& metalNames) {
    auto res = CalculateSimultaneousEquilibrium({ligandName}, {totalLigand}, metalNames, totalMetals);
    EquilibriumMultiResult legacyRes;
    legacyRes.freeLigand = res.freeLigands[0];
    legacyRes.totalLigand = totalLigand;
    legacyRes.metalNames = res.metalNames;
    legacyRes.freeMetals = res.freeMetals;
    legacyRes.totalMetals = res.totalMetals;
    legacyRes.complex = res.complexes[0];
    return legacyRes;
}

// Wrapper for legacy tests
EquilibriumResult CationSystem::CalculateConstrainedMulti(double totalLigand, const std::vector<double>& knownTotals,
                                               const std::vector<std::string>& knownNames, double targetFree,
                                               const std::string& targetName, const std::string& ligandName) {
    std::vector<std::string> mNames = knownNames;
    mNames.push_back(targetName);
    std::vector<double> mInputs = knownTotals;
    mInputs.push_back(targetFree);
    std::vector<bool> isFree(knownTotals.size(), false);
    isFree.push_back(true);

    auto res = CalculateSimultaneousEquilibrium({ligandName}, {totalLigand}, mNames, mInputs, isFree);
    
    EquilibriumResult legacyRes;
    legacyRes.freeLigand = res.freeLigands[0];
    
    // For legacy tests that expect secondary metal free conc in .freeMetal
    if (!knownTotals.empty()) {
        legacyRes.freeMetal = res.freeMetals[0]; 
    } else {
        legacyRes.freeMetal = res.freeMetals.back();
    }

    legacyRes.totalLigand = totalLigand;
    legacyRes.totalMetal = res.totalMetals.back();
    legacyRes.complex = res.complexes[0].back();
    return legacyRes;
}

EquilibriumResult CationSystem::CalculateEquilibrium(double totalLigand, double totalMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateTotalToFree(totalLigand, totalMetal, ligandName, metalName);
}

EquilibriumResult CationSystem::CalculateFreeToTotal(double totalLigand, double freeMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateFreeToTotal(totalLigand, freeMetal, ligandName, metalName);
}

EquilibriumResult CationSystem::CalculateTotalToFree(double totalLigand, double totalMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateTotalToFree(totalLigand, totalMetal, ligandName, metalName);
}

void CationSystem::AddLigand(const Ligand& ligand) {
    ligands.push_back(ligand);
}

void CationSystem::AddMetal(const Metal& metal) {
    metals.push_back(metal);
}

void CationSystem::Clear() {
    ligands.clear();
    metals.clear();
    adjustedConstants.clear();
}

std::vector<std::string> CationSystem::GetAvailableLigands() const {
    std::vector<std::string> names;
    for (const auto& ligand : ligands) names.push_back(ligand.name);
    return names;
}

std::vector<std::string> CationSystem::GetAvailableMetals() const {
    std::vector<std::string> names;
    for (const auto& metal : metals) names.push_back(metal.name);
    return names;
}

double CationSystem::GetAdjustedEquilibriumConstant(const std::string& ligandName, const std::string& metalName) {
    std::string key = ligandName + "_" + metalName;
    auto it = adjustedConstants.find(key);
    if (it != adjustedConstants.end()) return it->second;
    const Ligand* ligand = GetLigandByName(ligandName);
    if (!ligand) return 0.0;
    double adjustedK = std::pow(10.0, calculateStabilityConstant(ligand, metalName));
    adjustedConstants[key] = adjustedK;
    return adjustedK;
}

StabilityConstants CationSystem::GetStabilityConstants(const std::string& ligandName) const {
    const Ligand* ligand = GetLigandByName(ligandName);
    if (ligand) return ligand->constants;
    return StabilityConstants();
}

Metal CationSystem::GetMetalProperties(const std::string& metalName) const {
    const Metal* metal = GetMetalByName(metalName);
    if (metal) return *metal;
    return Metal();
}

void CationSystem::SetEquilibriumConstant(const std::string& key, double constant) {
    adjustedConstants[key] = constant;
}

double CationSystem::GetEquilibriumConstant(const std::string& key) const {
    auto it = adjustedConstants.find(key);
    if (it != adjustedConstants.end()) return it->second;
    return 0.0;
}

EquilibriumResult CationSystem::calculateFreeToTotal(double totalLigand, double freeMetal,
                                                   const std::string& ligandName, const std::string& metalName) {
    EquilibriumResult res;
    const Ligand* l = GetLigandByName(ligandName);
    if (!l) return res;
    double logK = calculateStabilityConstant(l, metalName);
    double K = std::pow(10.0, logK);
    double freeL = totalLigand / (1.0 + K * freeMetal);
    double c = K * freeMetal * freeL;
    res.freeLigand = freeL; res.freeMetal = freeMetal; res.complex = c;
    res.totalLigand = totalLigand; res.totalMetal = freeMetal + c;
    res.pH = params.pH; res.ionicStrength = params.ionicStrength;
    return res;
}

EquilibriumResult CationSystem::calculateTotalToFree(double totalLigand, double totalMetal,
                                                   const std::string& ligandName, const std::string& metalName) {
    EquilibriumResult res;
    const Ligand* l = GetLigandByName(ligandName);
    if (!l) return res;
    double logK = calculateStabilityConstant(l, metalName);
    double K = std::pow(10.0, logK);
    double c = solveForFreeMetal(totalMetal, totalLigand, K);
    res.freeLigand = totalLigand - c; res.freeMetal = totalMetal - c; res.complex = c;
    res.totalLigand = totalLigand; res.totalMetal = totalMetal;
    res.pH = params.pH; res.ionicStrength = params.ionicStrength;
    return res;
}
