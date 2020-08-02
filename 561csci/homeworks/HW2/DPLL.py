import copy
""" READ THE INPUT FILE """
inFile = open('CNF_sentences.txt', 'r')
outfile = open('CNF_satisfiability.txt', 'w')

def complimentryLiteral(temp_literal):
    if len(temp_literal[0]) == 1:
        comp_literal = ['not']
        comp_literal.append(temp_literal[0][0])
        return comp_literal
    elif len(temp_literal[0]) == 2:
        comp_literal = temp_literal[0][1]
        return comp_literal
    else:
        return temp_literal
        
def eliminateLiterals(rf_set, temp_set, comp_set):
    for check in range(0, len(rf_set)):
        for literals in range(0,len(rf_set[check])):
            new_set = copy.deepcopy(temp_set[0])
            if new_set == rf_set[check][literals]:
                del rf_set[check]
                break
            if comp_set == rf_set[check][literals]:
                del rf_set[check][literals]
                break
    return rf_set

def findUnitClause(rf_set):
    unit_clause = []
    if len(rf_set) > 0:
        for clause in range(0, len(rf_set)):
            if len(rf_set[clause]) == 1:
                unit_clause.append(rf_set[clause][0])
                break
            else:
                unit_clause = 0
    else:
        unit_clause = 0

    return unit_clause

def applyUnitClauseRule(rf_set, satisfiability_true):
    unit_clause = findUnitClause(rf_set)
    if unit_clause != 0:
        comp_clause = complimentryLiteral(unit_clause)
        if unit_clause in rf_set:
            rf_set.remove(unit_clause)
        rf_set = eliminateLiterals(rf_set, unit_clause, comp_clause)
        satisfiability_true.append(unit_clause)
        skip_recur = 0
        for clause in range(0, len(rf_set)):
            if len(rf_set[clause]) == 0:
                skip_recur = 1
        if skip_recur == 0:
            rf_set, satisfiability_true = applyUnitClauseRule(rf_set, satisfiability_true)
    return rf_set, satisfiability_true

def lookForPureSymbol(rf_set, comp_symbol):
    pure_symbol_id = 1
    for clause in range(0, len(rf_set)):
        for literals in range(0, len(rf_set[clause])):
            if comp_symbol == rf_set[clause][literals]:
                pure_symbol_id = 0
    return pure_symbol_id

def getPureSymbol(rf_set):
    pure_symbol_id = 0
    if len(rf_set) >= 0:
        for clause in range(0, len(rf_set)):
            for literals in range(0, len(rf_set[clause])):
                if rf_set[clause][literals] != 'not':
                    comp_symbol = []
                    comp_symbol.append(rf_set[clause][literals])
                    comp_symbol = complimentryLiteral(comp_symbol)
                    pure_symbol_id = lookForPureSymbol(rf_set, comp_symbol)
                    if pure_symbol_id == 1:
                        pure_symbol = rf_set[clause][literals]
                        return pure_symbol_id, pure_symbol
        if pure_symbol_id == 0:
            return 0,0
    else:
        return 0,0
   
def removePureSymbolClauses(rf_set, pure_symb):
    new_set = []
    for check in range(0, len(rf_set)):
        found = 0
        for literals in range(0,len(rf_set[check])):
            if pure_symb == rf_set[check][literals]:
                found = 1
        if found == 0:
            new_set.append(rf_set[check])
    return new_set

def applyPureSymbolsRule(rf_set, satisfiability_true):
    skip = 0
    if len(rf_set) == 1:
        if rf_set[0][0] == 'not':
            satisfiability_true.append(rf_set[0])
            skip = 1
        
    if len(rf_set) > 0 and skip == 0:
        pure_symbol_id, pure_symbol = getPureSymbol(rf_set)
        if pure_symbol_id == 1:
            rf_set = removePureSymbolClauses(rf_set, pure_symbol)
            new_set = []
            new_set.append(pure_symbol)
            satisfiability_true.append(new_set)

            rf_set, satisfiability_true = applyPureSymbolsRule(rf_set, satisfiability_true)
    return rf_set, satisfiability_true

def obtainTestLiteral(rf_set, checked):
    test_literal = 0
    for clause in range(0, len(rf_set)):
        for literals in range(0, len(rf_set[clause])):
            if rf_set[clause][literals] not in checked:
                test_literal = rf_set[clause][literals]
                return test_literal
    return test_literal

def implementUnitClause(rf_set, unit_clause, satisfiability_true):
    if unit_clause == 0:
        temp_clause = findUnitClause(rf_set)
        if temp_clause != 0:
            if len(temp_clause) == 1:
                unit_clause = copy.deepcopy(temp_clause[0])
            else:
                unit_clause = copy.deepcopy(temp_clause)
        else:
                unit_clause = copy.deepcopy(temp_clause)
    if unit_clause != 0:
        new_clause = []
        new_clause.append(unit_clause)
        comp_clause = complimentryLiteral(new_clause)
        new_set = []
        satisfiability_true.append(unit_clause)  
        for check in range(0, len(rf_set)):
            found = 0
            for literals in range(0,len(rf_set[check])):
                if unit_clause == rf_set[check][literals]:
                    found = 1
                    break
            if found == 0:
                new_set.append(rf_set[check])
        new_set1 = []
        for check in range(0, len(new_set)):
            found = 0
            temp_clause = []
            for literals in range(0,len(new_set[check])):
                if comp_clause != new_set[check][literals]:
                    temp_clause.append(new_set[check][literals])
                    found = 1
            new_set1.append(temp_clause)
        skip_recur = 0
        if len(new_set1) > 0:
            for clause in range(0, len(new_set1)):
                if len(new_set1[clause]) == 0:
                    skip_recur = 1
                    return new_set1
        else:
            skip_recur = 1
            return new_set1
        if skip_recur == 0:
            rf_set = implementUnitClause(new_set1, 0, satisfiability_true)
    else:
        new_set1 = copy.deepcopy(rf_set)
        return new_set1

def implementSplitUnitRule(rf_set, checked, satisfiability_true):
    test_literal = obtainTestLiteral(rf_set, checked)
    if test_literal != 0:
        checked.append(test_literal)
        rf_set = implementUnitClause(rf_set, test_literal, satisfiability_true)
        if not rf_set:
            print 'Empty sentence'
        else:
            rf_set, satisfiability_true = implementSplitUnitRule(rf_set, checked, satisfiability_true)
    return rf_set, satisfiability_true

def applySplittingRule(rf_set, satisfiability_true):
    checked = []
    rf_set, satisfiability_true = implementSplitUnitRule(rf_set, checked, satisfiability_true)
    return rf_set, satisfiability_true

def convertCNFToSets(sentence):
    rf_set = []
    if sentence[0] == 'and':
        for clause in range(1, len(sentence)):
            if sentence[clause][0] == 'or':
                new_set = []
                for literal in range(1,len(sentence[clause])):
                    new_set.append(sentence[clause][literal])
                rf_set.append(new_set)
            else:
                temp = []
                temp.append(sentence[clause])
                rf_set.append(temp)
    elif sentence[0] == 'not':
        rf_set.append(sentence)
    else:
        new_set = []
        for clause in range(1, len(sentence)):
            new_set.append(sentence[clause])
        rf_set.append(new_set)
    return rf_set

def getUniqueLiterals(rf_set):
    literalsList = []
    for clause in range(0, len(rf_set)):
        for literals in range(0, len(rf_set[clause])):
            if rf_set[clause][literals] != 'not':
                for notlit in range(0, len(rf_set[clause][literals])):
                    if rf_set[clause][literals][notlit] != 'not':
                        if rf_set[clause][literals][notlit] not in literalsList:
                            literalsList.append(rf_set[clause][literals][notlit])
    return literalsList

def obtainFinalResult(satisfiability_true, literals_list, emptyClause):
    result = []
    if emptyClause == 1:
        result.append('false')
    else:
        result.append('true')
        for clause in range(len(literals_list)):
            comp_literal = complimentryLiteral(literals_list[clause])
            temp_literal = []
            temp_literal.append(literals_list[clause])
            print_literal = 0
            for i in range(len(satisfiability_true)):
                temp_literal1 = []
                temp_literal1.append(temp_literal)
                comp_literal1 = []
                comp_literal1.append(comp_literal)
                if (temp_literal == satisfiability_true[i]) or (temp_literal1 == satisfiability_true[i]):
                    result.append(literals_list[clause]+'=true')
                    print_literal = 1
                elif (comp_literal == satisfiability_true[i]) or (comp_literal1 == satisfiability_true[i]):
                    result.append(literals_list[clause]+'=false')
                    print_literal = 1
            if print_literal == 0:
                result.append(literals_list[clause]+'=true')
    return result

def doSatisfiabilityCheck(rf_set, satisfiability_true, literals_list):
    emptyClause = 0
    completed = 0
    if not rf_set:
        result = obtainFinalResult(satisfiability_true, literals_list, emptyClause)
    else:
        for clause in range(len(rf_set)):
            if len(rf_set[clause]) == 0:
                emptyClause = 1
                completed = 1
        if emptyClause == 1:
            result = obtainFinalResult(satisfiability_true, literals_list, emptyClause)
            completed = 1
        else:
            return completed, 0
    return completed, result

for instr in inFile:
    sentence = eval(instr)
    satisfiability_true = []
    literals_list = []
    if instr[0] == '[':
        rf_set = convertCNFToSets(sentence)
        if len(rf_set) > 1:
            rf_set.sort(key = len)
        literals_list = []
        literals_list = getUniqueLiterals(rf_set)
        rf_set, satisfiability_true = applyUnitClauseRule(rf_set,satisfiability_true)
        completed = 0
        completed, result = doSatisfiabilityCheck(rf_set, satisfiability_true, literals_list)
        if completed == 0:
            rf_set, satisfiability_true = applyPureSymbolsRule(rf_set,satisfiability_true)
            completed, result = doSatisfiabilityCheck(rf_set, satisfiability_true, literals_list)
        if completed == 0:
            rf_set, satisfiability_true = applySplittingRule(rf_set, satisfiability_true)
            completed, result = doSatisfiabilityCheck(rf_set, satisfiability_true, literals_list)
        outfile.write("%s\n" %result)
inFile.close()
outfile.close()
