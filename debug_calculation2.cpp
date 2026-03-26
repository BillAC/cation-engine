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
    double logK = 10.97; // EGTA-Ca2+ from CSV
    double total_L = 10.0; // mM
    double total_Ca = 5.0; // mM
    
    double alpha = calculateAlpha(pH);
    double gamma_Ca = ionicCorrection(I, 2.0);
    double gamma_EGTA = ionicCorrection(I, 4.0);
    
    std::cout << "Alpha: " << alpha << " (" << std::log10(alpha) << " log)" << std::endl;
    std::cout << "Gamma_Ca: " << gamma_Ca << " (" << std::log10(gamma_Ca) << " log)" << std::endl;
    std::cout << "Gamma_EGTA: " << gamma_EGTA << " (" << std::log10(gamma_EGTA) << " log)" << std::endl;
    
    double K_thermo = std::pow(10, logK);
    std::cout << "K_thermo: " << K_thermo << " (" << logK << " log)" << std::endl;
    
    // Conditional constant K' = K_thermo / (gamma_Ca * gamma_EGTA)
    double K_prime = K_thermo / (gamma_Ca * gamma_EGTA);
    std::cout << "K': " << K_prime << " (" << std::log10(K_prime) << " log)" << std::endl;
    
    // Effective constant K'' = K' * alpha
    double K_eff = K_prime * alpha;
    std::cout << "K_eff: " << K_eff << " (" << std::log10(K_eff) << " log)" << std::endl;
    
    // Solve quadratic: complex = K_eff * (total_Ca - complex) * (total_L - complex)
    // Let x = complex
    // x = K_eff * (total_Ca - x) * (total_L - x)
    // x = K_eff * (total_Ca*total_L - total_Ca*x - total_L*x + x*x)
    // x - K_eff*total_Ca*total_L + K_eff*total_Ca*x + K_eff*total_L*x - K_eff*x*x = 0
    // -K_eff*x*x + (K_eff*total_Ca + K_eff*total_L + 1)*x - K_eff*total_Ca*total_L = 0
    
    double A = -K_eff;
    double B = K_eff*total_Ca + K_eff*total_L + 1;
    double C = -K_eff*total_Ca*total_L;
    
    double discriminant = B*B - 4*A*C;
    std::cout << "Discriminant: " << discriminant << std::endl;
    
    if (discriminant < 0) {
        std::cout << "No real solution" << std::endl;
        return 1;
    }
    
    double sqrtD = std::sqrt(discriminant);
    double x1 = (-B + sqrtD) / (2*A);
    double x2 = (-B - sqrtD) / (2*A);
    
    std::cout << "x1: " << x1 << ", x2: " << x2 << std::endl;
    
    // Choose the physically reasonable solution
    double complex = -1;
    if (x1 >= 0 && x1 <= std::min(total_Ca, total_L)) complex = x1;
    else if (x2 >= 0 && x2 <= std::min(total_Ca, total_L)) complex = x2;
    
    if (complex < 0) {
        std::cout << "No valid solution found" << std::endl;
        return 1;
    }
    
    double free_Ca = total_Ca - complex;
    double free_L = total_L - complex;
    
    std::cout << "Complex: " << complex << " mM" << std::endl;
    std::cout << "Free Ca: " << free_Ca * 1e6 << " nM" << std::endl;
    std::cout << "Free L: " << free_L << " mM" << std::endl;
    
    // Check equilibrium
    double check_complex = K_eff * free_Ca * free_L;
    std::cout << "Check complex: " << check_complex << " mM" << std::endl;
    std::cout << "Difference: " << std::abs(check_complex - complex) << std::endl;
    
    return 0;
}
