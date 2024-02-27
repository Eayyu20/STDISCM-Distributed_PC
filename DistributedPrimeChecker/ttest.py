 
# Importing library 
import scipy.stats as stats 
  
singlemachine = [2.124, #1
                1.3184, #2
                0.7152, #4
                0.3972, #8
                0.291, #16
                0.331, #32
                0.2742, #64
                0.2636, #128
                0.2622, #256
                0.2642, #512
                0.2746] #1024
  
#insert tested values here
distributed =   [2.124, #1
                1.3184, #2
                0.7152, #4
                0.3972, #8
                0.291, #16
                0.331, #32
                0.2742, #64
                0.2636, #128
                0.2622, #256
                0.2642, #512
                0.2746] #1024
  
# Performing the paired sample t-test 
results = stats.ttest_rel(singlemachine, distributed) 
print(results)