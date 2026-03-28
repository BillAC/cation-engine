#include <iostream>
#include <cmath>

// Multi-step protonation Henderson-Hasselbalch
double calculateProtonationFraction(double pH, double pK1, double pK2, double pK3, double pK4) {
    // Denominator sums all protonation states
    double D = 1.0; // Ln- (fully deprotonated, no protons)
    
    // Add HLn-1 state (one proton bound)
    if (pK1 > 0) D += std::pow(10, pK1 - pH);
    
    // Add H2Ln-2 state (two protons bound)
    if (pK1 > 0 && pK2 > 0) D += std::pow(10, pK1 + pK2 - 2*pH);
    
    // Add H3Ln-3 state (three protons bound)
    if (pK1 > 0 && pK2 > 0 && pK3 > 0) D += std::pow(10, pK1 + pK2 + pK3 - 3*pH);
    
    // Add H4Ln-4 state (four protons bound)
    if (pK1 > 0 && pK2 > 0 && pK3 > 0 && pK4 > 0) D += std::pow(10, pK1 + pK2 + pK3 + pK4 - 4*pH);
    
    // Fraction of fully deprotonated form: alpha = 1/D
    return 1.0 / D;
}

int main() {
    // EGTA pK values
    double pK1 = 9.4, pK2 = 8.79, pK3 = 2.68, pK4 = 1.93;
    double pH = 7.2;
    
    double alpha = calculateProtonationFraction(pH, pK1, pK2, pK3, pK4);
    std::cout << "EGTA protonation fraction at pH " << pH << ": " << alpha << "\n";
    std::cout << "log10(alpha): " << std::log10(alpha) << "\n";
    
    // EDTA pK values
    double pK1_edta = 10.17, pK2_edta = 6.11, pK3_edta = 2.68, pK4_edta = 2.0;
    double alpha_edta = calculateProtonationFraction(pH, pK1_edta, pK2_edta, pK3_edta, pK4_edta);
    std::cout << "EDTA protonation fraction at pH " << pH << ": " << alpha_edta << "\n";
    std::cout << "log10(alpha): " << std::log10(alpha_edta) << "\n";
    
    return 0;
}
