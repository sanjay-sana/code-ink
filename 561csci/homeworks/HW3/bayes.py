import sys
import copy
from collections import defaultdict
from timeit import itertools

inFile = open(sys.argv[2])
outFile = open('sample_input_inference.txt', 'w')

""" Reads input from the file - line by line"""
""" Extract count of number of patients/diseases """
line1 = inFile.readline()
count = line1.strip().split(' ')
""" count[0] = no. of diseases; count[1] = no. of patients """
diseases = []
prob_diseases = defaultdict(list)
symptoms = defaultdict(list)
p_givenDisease = defaultdict(list)
p_givenNoDisease = defaultdict(list)

""" Extract data related to each of the diseases """
for i in range(int(count[0])):
    temp_line = inFile.readline()
    disease_list = temp_line.strip().split(' ')
    diseases.append(disease_list[0])
    prob_diseases[diseases[i]] = disease_list[2]
    
    temp = eval(inFile.readline())
    temp_list = []
    for j in range(int(disease_list[1])):
        temp_list.append(temp[j])
    symptoms[diseases[i]] = temp_list
    
    temp = eval(inFile.readline())
    temp_list1 = []
    for j in range(int(disease_list[1])):
        temp_list1.append(temp[j])
    p_givenDisease[diseases[i]] = temp_list1
    
    temp = eval(inFile.readline())
    temp_list2 = []
    for j in range(int(disease_list[1])):
        temp_list2.append(temp[j])
    p_givenNoDisease[diseases[i]] = temp_list2

"""Extract probability values from the respective dictionary"""
def getProbability(currentDisease, p_givenDisease, Symb):
#     print 'current',p_givenDisease[currentDisease]
    values = []
    for i in range(len(Symb)):
        values.append(p_givenDisease[currentDisease][Symb[i]])
    return values

"""Calculate the final probability P[disease | Symptoms] using Bayes theorem - Questions 1"""
def getProbabilityGivenDisease(currentDisease, valuesTrue, valuesTrueNot, valuesFalse, valuesFalseNot, prob_diseases):
    numerator = 1
    for i in range(len(valuesTrue)):
        numerator *= valuesTrue[i]
    for i in range(len(valuesFalse)):
        numerator *= (1 - valuesFalse[i])
    numerator *= float(prob_diseases[currentDisease])
#     print 'numerator:', numerator
    denominator2 = 1
    for i in range(len(valuesTrueNot)):
        denominator2 *= valuesTrueNot[i]
    for i in range(len(valuesFalseNot)):
        denominator2 *= (1 - valuesFalseNot[i])
    denominator2 *= (1 - float(prob_diseases[currentDisease]))
    denominator = numerator + denominator2
#     print 'denominator:', denominator
    result = numerator/denominator
#     print 'result:', result
    return result, numerator, denominator2

"""Calculate the final probability P[disease | Symptoms] using Bayes theorem - Questions 2"""
def getProbabilityUnavail(numerator, denominator2, valuesUnaT, valuesUnaTNot, valuesUnaF, valuesUnaFNot):
    for i in range(len(valuesUnaT)):
        numerator *= valuesUnaT[i]
    for i in range(len(valuesUnaF)):
        numerator *= (1 - valuesUnaF[i])
#     print 'numerator:', numerator
#     denominator2 = 1
    for i in range(len(valuesUnaTNot)):
        denominator2 *= valuesUnaTNot[i]
    for i in range(len(valuesUnaFNot)):
        denominator2 *= (1 - valuesUnaFNot[i])
    denominator = numerator + denominator2
#     print 'denominator:', denominator
    result = numerator/denominator
#     print 'result:', result
    return result

"""Convert Question-1 result dictionary into string"""
def format1(d, tab=0):
    s = []
    for k,v in d.items():
        s.append("%s%r: '%s'" % ('  '*tab, k, v))
    return ', '.join(s)

"""Convert Question-2,3 result dictionary into string"""
def format2(d, tab=0):
    s = []
    for k,v in d.items():
        s.append('%s%r: %s' % ('  '*tab, k, v))
    return ', '.join(s)

"""Process values for all diseases for each of the patient"""
for m in range(int(count[1])):
    result_dict = dict()
    range_dict = dict()
    next_dict = dict()
    for n in range(int(count[0])):
#         print symptoms[diseases[n]]
        noFindings = 0
        unavailable = 0
        probabilityRange =[]
        current = eval(inFile.readline())
################## Question 1 #####################
        allTrue = [i for i, x in enumerate(current) if x == "T"]
        valuesTrue = getProbability(diseases[n], p_givenDisease, allTrue)
        valuesTrueNot = getProbability(diseases[n], p_givenNoDisease, allTrue)
#         print 'probT:', valuesTrue, valuesTrueNot
        allFalse = [i for i, x in enumerate(current) if x == "F"]
        valuesFalse = getProbability(diseases[n], p_givenDisease, allFalse)
        valuesFalseNot = getProbability(diseases[n], p_givenNoDisease, allFalse)
#         print 'probF:', valuesFalse, valuesFalseNot
        result, numerator, denominator2 = getProbabilityGivenDisease(diseases[n], valuesTrue, valuesTrueNot, valuesFalse, valuesFalseNot, prob_diseases)
#         result = round(result,4)
        result = format(result, '.4f')
        result_dict[diseases[n]] = result
################## Question 2 #####################
        allUnavailable = [i for i, x in enumerate(current) if x == "U"]
#         print 'all:', allUnavailable
        table = list(itertools.product(['T', 'F'], repeat=len(allUnavailable)))
        for k in range(len(table)):
            allUnaTrue = [i for i, x in enumerate(table[k]) if x == "T"]
            trueIndexes = []
            for s in range(len(allUnaTrue)):
                trueIndexes.append(allUnavailable[allUnaTrue[s]])
#             print 'truevalues', trueIndexes
            valuesUnaT = getProbability(diseases[n], p_givenDisease, trueIndexes)
            valuesUnaTNot = getProbability(diseases[n], p_givenNoDisease, trueIndexes)
#             print 'T:', valuesUnaT, valuesUnaTNot
            allUnaFalse = [i for i, x in enumerate(table[k]) if x == "F"]
            falseIndexes = []
            for s in range(len(allUnaFalse)):
                falseIndexes.append(allUnavailable[allUnaFalse[s]])
            valuesUnaF = getProbability(diseases[n], p_givenDisease, falseIndexes)
            valuesUnaFNot = getProbability(diseases[n], p_givenNoDisease, falseIndexes)  
#             print 'F:', valuesUnaF, valuesUnaFNot
            if (len(trueIndexes) + len(falseIndexes)):
                tempProb = getProbabilityUnavail(numerator, denominator2, valuesUnaT, valuesUnaTNot, valuesUnaF, valuesUnaFNot)
#                 tempProb = round(tempProb,4)
                tempProb = format(tempProb, '.4f')
                probabilityRange.append(tempProb)
                noFindings = 1
################## Question 3 #####################
#         print allUnavailable
        symptomCases = ['T', 'F']
        newResult = []
        for s in range(len(symptomCases)):
            for t in range(len(allUnavailable)):
                newSymptoms = copy.deepcopy(current)
                newSymptoms[allUnavailable[t]] = copy.deepcopy(symptomCases[s])
#                 print 'new:',newSymptoms
                all_true = [i for i, x in enumerate(newSymptoms) if x == "T"]
                values_true = getProbability(diseases[n], p_givenDisease, all_true)
                values_trueNot = getProbability(diseases[n], p_givenNoDisease, all_true)
                
                all_false = [i for i, x in enumerate(newSymptoms) if x == "F"]
                value_false = getProbability(diseases[n], p_givenDisease, all_false)
                value_falseNot = getProbability(diseases[n], p_givenNoDisease, all_false)

                res, numer, denom2 = getProbabilityGivenDisease(diseases[n], values_true, values_trueNot, value_false, value_falseNot, prob_diseases)
#                 res = round(res,4)
                res = format(res, '.4f')
                newResult.append(res)
                unavailable = 1
#         print 'res: ', diseases[n], newResult
        tList = []
        if noFindings == 1:
            tList.append(min(probabilityRange))
            tList.append(max(probabilityRange))
            range_dict[diseases[n]] = tList
        else:
            tList.append(result_dict[diseases[n]])
            tList.append(result_dict[diseases[n]])
            range_dict[diseases[n]] = tList
        frameList = []
#         print newResult
        if unavailable == 1:
            allMax = [i for i, x in enumerate(newResult) if x == max(newResult)]
            if len(allMax) == 1:
                maxResult_index = newResult.index(max(newResult))
            else:
#                 for a in range(len(allMax)):
#                 tempIdx = allMax[a]
                maxSymptomOrder = []
                for a in range(len(allMax)):
                    if allMax[a] < len(allUnavailable):
                        tempIdx = allUnavailable[allMax[a]]
#                         maxSymptomOrder.append(symptoms[diseases[n]][tempIdx])
                    else:
                        tempIdx = allUnavailable[allMax[a] - len(allUnavailable)]
                    maxSymptomOrder.append(symptoms[diseases[n]][tempIdx])
#                 maxSymptomOrder.sort()
                zip2List = zip(maxSymptomOrder, allMax)
                allMax_sorted = [t1 for t, t1 in sorted(zip2List)]
                maxResult_index = allMax_sorted[0]
            if maxResult_index < len(allUnavailable):
                if max(newResult) > result_dict[diseases[n]]:
#                     allMax = [i for i, x in enumerate(newResult) if x == max(newResult)]
#                     if len(allMax) == 1:
                    origIdx = allUnavailable[maxResult_index]
                    frameList.append(symptoms[diseases[n]][origIdx])
                    frameList.append('T')
                else:
                    frameList.append('none')
                    frameList.append('N')
            else:
                if max(newResult) > result_dict[diseases[n]]:
                    origIdx = allUnavailable[maxResult_index - len(allUnavailable)]
                    frameList.append(symptoms[diseases[n]][origIdx])
                    frameList.append('F')
                else:
                    frameList.append('none')
                    frameList.append('N')
            allMin = [i for i, x in enumerate(newResult) if x == min(newResult)]
            if len(allMin) == 1:
                minResult_index = newResult.index(min(newResult))
            else:
                minSymptomOrder = []
                for a in range(len(allMin)):
                    if allMin[a] < len(allUnavailable):
                        tempIdx = allUnavailable[allMin[a]]
                    else:
                        tempIdx = allUnavailable[allMin[a] - len(allUnavailable)]
                    minSymptomOrder.append(symptoms[diseases[n]][tempIdx])
                zip2List1 = zip(minSymptomOrder, allMin)
                allMin_sorted = [t1 for t, t1 in sorted(zip2List1)]
                minResult_index = allMin_sorted[0]
            if minResult_index < len(allUnavailable):
                if min(newResult) < result_dict[diseases[n]]:
                    origIdx = allUnavailable[minResult_index]
                    frameList.append(symptoms[diseases[n]][origIdx])
                    frameList.append('T')
                else:
                    frameList.append('none')
                    frameList.append('N')
            else:
    #             print 'yes2'
                if min(newResult) < result_dict[diseases[n]]:
                    origIdx = allUnavailable[minResult_index - len(allUnavailable)]
                    frameList.append(symptoms[diseases[n]][origIdx])
                    frameList.append('F')
                else:
                    frameList.append('none')
                    frameList.append('N')
    #             print 'yes2:',frameList
            next_dict[diseases[n]] = frameList
        else:
            for z in range(2):
                frameList.append('none')
                frameList.append('N')
                next_dict[diseases[n]] = frameList
        
#     print 'Patient-%s:' %(m+1)
#     print result_dict
#     print range_dict
#     print next_dict
############# Write final processed data into the outfile #####################
    outFile.write('Patient-%s:\n' %(m+1))
    temp_str = format1(result_dict)
    outFile.write('{'+temp_str+'}'+'\n');
    temp_str = format2(range_dict)
    outFile.write('{'+temp_str+'}'+'\n');
    temp_str = format2(next_dict)
    outFile.write('{'+temp_str+'}'+'\n');
