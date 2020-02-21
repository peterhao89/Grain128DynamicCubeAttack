These are all the source codes for verifying our results. To compile them, one need to have licensed Gurobi. After modifying the directory of Gurobi in "makefile", one can compile all our experiments as:
make all
The experiments include:
1. Grain128Tests: all verification expriments related to our dynamic cube attack on Grain-128.
2. AcornTests: determine all parameters namely J, \tilde{J}^2 and minimized \tilde{J}^1 for Acorn. 
3. KreyviumTests: determine all parameters namely J, \tilde{J}^2 and minimized \tilde{J}^1 for Kreyvium. 
4. *Bounds: determine the secure bounds for the corresponding Grain-like stream ciphers against zero-sum & bias cube testers. 