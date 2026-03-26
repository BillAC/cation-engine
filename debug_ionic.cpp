#include <iostream>
#include <cmath>

// Davies equation parameters
const double A = 0.51; // Temperature-dependent parameter (0.51 at 25°C)

double calculateIonicStrengthCorrection(double ionicStrength, double charge) {
    if (ionicStrength <= 0) return 1.0;

    double sqrtI = std::sqrt(ionicStrength);
    
    // Davies equation: log(γ) = -A*z²*[√I/(1+√I) - 0.3*I]
    double logGamma = -A * charge * charge * 
                     (sqrtI / (1.0 + sqrtI) - 0.3 * ionicStrength);
    
    // Activity coefficient: γ = 10^(logGamma)
    return std::pow(10, logGamma);
}

int main() {
    double ionicStrength = 100.0; // 100 mM = 0.1 M
    double charge = 2.0;
    
    double correction = calculateIonicStrengthCorrection(ionicStrength, charge);
    std::cout << "Ionic strength: " << ionicStrength << " mM\n";
    std::cout << "Charge: " << charge << "\n";
    std::cout << "Correction factor: " << correction << "\n";
    std::cout << "log10(correction): " << std::log10(correction) << "\n";
    
    // Test with different ionic strengths
    for (double I : {0.0, 10.0, 100.0, 1000.0}) {
        double corr = calculateIonicStrengthCorrection(I, 2.0);
        std::cout << "I=" << I << " mM: correction=" << corr << ", log10=" << std::log10(corr) << "\n";
    }
    
    return 0;
}
