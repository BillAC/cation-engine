#include <iostream>
#include <cmath>

// Test case 1: 10 mM EGTA, 5 mM total Ca2+, pH 7.2, I=100 mM
// Expected: free Ca2+ = 67.8 nM

// EGTA pK values: 9.4, 8.79, 2.68, 1.93
double calculateAlpha(double pH) {
    double pK1 = 9.4, pK2 = 8.79, pK3 = 2.68, pK4 = 1.93;
    double D = 1.0;
    if (pK1 > 0) D += std::pow(10, pK1 - pH);
    if (pK1 > 0 && pK2 > 0) D += std::pow(10, pK1 + pK2 - 2*pH);
    if (pK1 > 0 && pK2 > 0 && pK3 > 0) D += std::pow(10, pK1 + pK2 + pK3 - 3*pH);
    if (pK1 > 0 && pK2 > 0 && pK3 > 0 && pK4 > 0) D += std::pow(10, pK1 + pK2 + pK3 + pK4 - 4*pH);
    return 1.0 / D;
}

// Davies equation for ionic strength correction
double ionicCorrection(double I_mM, double charge) {
    double I = I_mM / 1000.0; // mM to M
    double A = 0.51;
    double sqrtI = std::sqrt(I);
    double logGamma = -A * charge * charge * (sqrtI / (1.0 + sqrtI) - 0.3 * I);
    return std::pow(10, logGamma);
}

int main() {
    double pH = 7.2;
    double I = 100.0; // mM
    double logK = 11.0; // EGTA-Ca2+
    double total_L = 10.0; // mM
    double total_Ca = 5.0; // mM
    
    double alpha = calculateAlpha(pH);
    double gamma_Ca = ionicCorrection(I, 2.0);
    double gamma_EGTA = ionicCorrection(I, 4.0); // EGTA has charge -4 when deprotonated
    
    std::cout << "Alpha (fraction deprotonated): " << alpha << " (" << std::log10(alpha) << " log)" << std::endl;
    std::cout << "Gamma_Ca: " << gamma_Ca << " (" << std::log10(gamma_Ca) << " log)" << std::endl;
    std::cout << "Gamma_EGTA: " << gamma_EGTA << " (" << std::log10(gamma_EGTA) << " log)" << std::endl;
    
    // The thermodynamic K is for: Ca2+ + EGTA4- ⇌ CaEGTA2-
    // But activity coefficients apply to the free ions
    // So K_thermodynamic = [CaEGTA]/[Ca_free * EGTA_free] = K * gamma_Ca * gamma_EGTA
    // The logK in tables is usually the thermodynamic constant
    
    double K_thermo = std::pow(10, logK);
    std::cout << "K_thermodynamic: " << K_thermo << " (" << logK << " log)" << std::endl;
    
    // For the calculation, we need to solve:
    // total_L = L_free + complex
    // total_Ca = Ca_free + complex
    // complex = K_thermo * Ca_free * L_free / (gamma_Ca * gamma_EGTA)
    // Because K_thermo = [complex] / ([Ca_free] * gamma_Ca * [L_free] * gamma_EGTA)
    
    // Actually, the standard way is:
    // The conditional constant K' = K_thermo / (gamma_Ca * gamma_EGTA)
    // And K' = [complex] / ([Ca_free] * [L_free])
    
    double K_prime = K_thermo / (gamma_Ca * gamma_EGTA);
    std::cout << "K': " << K_prime << " (" << std::log10(K_prime) << " log)" << std::endl;
    
    // But we also have protonation. The total ligand includes protonated forms.
    // The effective free ligand concentration is alpha * total_L_free
    // So the effective K'' = K' * alpha
    
    double K_double_prime = K_prime * alpha;
    std::cout << "K'': " << K_double_prime << " (" << std::log10(K_double_prime) << " log)" << std::endl;
    
    // Now solve the quadratic for complex concentration
    // complex = K'' * (total_Ca - complex) * (alpha * (total_L - complex))
    // Let x = complex
    // x = K'' * (total_Ca - x) * alpha * (total_L - x)
    // x = K'' * alpha * (total_Ca - x) * (total_L - x)
    
    double a = K_double_prime * alpha;
    double b = total_Ca;
    double c = total_L;
    
    // x = a * (b - x) * (c - x)
    // x = a*(b*c - b*x - c*x + x*x)
    // x = a*b*c - a*b*x - a*c*x + a*x*x
    // 0 = a*x*x - (a*b + a*c - 1)*x + a*b*c
    
    double A = a;
    double B = -(a*b + a*c - 1);
    double C = a*b*c;
    
    double discriminant = B*B - 4*A*C;
    if (discriminant < 0) {
        std::cout << "No real solution" << std::endl;
        return 1;
    }
    
    double x1 = (-B + std::sqrt(discriminant)) / (2*A);
    double x2 = (-B - std::sqrt(discriminant)) / (2*A);
    
    double complex = (x1 >= 0 && x1 <= std::min(b, c)) ? x1 : x2;
    double free_Ca = total_Ca - complex;
    double free_L_total = total_L - complex;
    double free_L_deprotonated = alpha * free_L_total;
    
    std::cout << "Complex: " << complex << " mM" << std::endl;
    std::cout << "Free Ca: " << free_Ca * 1e6 << " nM" << std::endl;
    std::cout << "Free L total: " << free_L_total << " mM" << std::endl;
    std::cout << "Free L deprotonated: " << free_L_deprotonated << " mM" << std::endl;
    
    // Check equilibrium
    double check_complex = K_prime * free_Ca * free_L_deprotonated;
    std::cout << "Check complex: " << check_complex << " mM" << std::endl;
    
    return 0;
}
