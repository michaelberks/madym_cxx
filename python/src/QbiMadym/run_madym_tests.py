import sys
from QbiMadym import madym_DCE, madym_DCE_lite, madym_T1
def run_madym_tests(test_level=1, plot_output=True):
    '''RUN_MADYM_TESTS run set of tests tests on C++ madym_DCE tools.
       [] = run_madym_tests()
    
     Inputs:
           test_level (int, >= 1) - set level of tests required.
               1 = Basic tests only (currently just this implemented)
            
            plot_output (Bool) - if true, show test plots. Set to false to run non-interactively
                eg for installing on CSF
     Outputs:
    
     Example:
    
     Notes:
    
     See also: INSTALL_MADYM
    
     Created: 01-May-2019
     Author: Michael Berks 
     Email : michael.berks@manchester.ac.uk 
     Phone : +44 (0)161 275 7669 
     Copyright: (C) University of Manchester'''
    print(f'*****************************************************')
    print(f'Running madym_DCE tests - level {test_level}')
    print(f'*****************************************************')

    #Calling functions with no input runs basic tests
    madym_DCE_lite.test(plot_output)
    madym_DCE.test(plot_output)
    madym_T1.test(plot_output)

    if test_level > 1:
        #Apply extended tests
        pass

    print(f'*****************************************************')
    print(f'All tests passed')
    print(f'*****************************************************')

if __name__ == '__main__':
    if len(sys.argv) > 1:
        level = int(sys.argv)
    else:
        level = 1
    run_madym_tests(level)

