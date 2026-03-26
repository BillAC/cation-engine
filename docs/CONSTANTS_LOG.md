# Constants Discrepancy Log

## 2026-03-25
- Source: `ligands.csv` (project SoT)
- Checked: EDTA Ca1 logK = 10.65; site uses 10.65, in agreement with NIST/IUPAC.
- Checked: EGTA Ca1 logK = 10.97; site uses 10.97, in agreement with WebMaxC reference.
- Note: No differences >0.05 logK observed for tested interactions (EDTA and EGTA with Ca2+).  
- Implementation: `CationSystem::calculateStabilityConstant()` now supports pH/ionic/temperature corrections.
