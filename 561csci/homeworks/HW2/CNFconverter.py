import copy

""" READ THE INPUT FILE """
inFile = open('sentences.txt', 'r')
outfile = open('sentences_CNF.txt', 'w')

""" Reformats when iff is found """
def eliminateIff(sentence):   
    temp_sent = copy.deepcopy(sentence)
    temp_sent[0] = 'and'
    temp_clause = ['implies']
    temp_clause.append(sentence[1])
    temp_clause.append(sentence[2])
    temp_sent[1] = temp_clause
    temp_clause = ['implies']
    temp_clause.append(sentence[2])
    temp_clause.append(sentence[1])
    temp_sent[2] = temp_clause
    return temp_sent

""" Reformats when implication is found """
def eliminateImplication(sentence):
    temp_sent = copy.deepcopy(sentence)
    temp_sent[0] = 'or'
    temp_clause = ['not']
    temp_clause.append(sentence[1])
    temp_sent[1] = temp_clause
    return temp_sent

""" Eliminates nots when found next to each other """
def applyDoubleNegetion(sentence):
    temp_sent = copy.deepcopy(sentence[1][1])
    return temp_sent

""" Applying DeMorgans rule to move nots inside """
def applyDeMorgans(sentence):
    new_sent = []
    if sentence[1][0] == 'and':
        new_sent.append('or')
        temp_clause = ['not']
        temp_clause.append(sentence[1][1])
        new_sent.append(temp_clause)
        temp_clause1 = ['not']
        temp_clause1.append(sentence[1][2])
        new_sent.append(temp_clause1)
        return new_sent
    elif sentence[1][0] == 'or':
        new_sent.append('and')
        temp_clause = ['not']
        temp_clause.append(sentence[1][1])
        new_sent.append(temp_clause)
        temp_clause1 = ['not']
        temp_clause1.append(sentence[1][2])
        new_sent.append(temp_clause1)
        return new_sent
    else:
        return sentence

""" Applying distributive laws """
def applyDistributive(sentence):
    temp_sent = []
    if sentence[0] == 'or' and sentence[1][0] == 'and' and sentence[2][0] == 'and':
        temp_sent.append('and')
        temp_clause1 = ['or']
        temp_clause1.append(sentence[1][1])
        temp_clause1.append(sentence[2])
        temp_sent.append(temp_clause1)
        temp_clause2 = ['or']
        temp_clause2.append(sentence[1][2])
        temp_clause2.append(sentence[2])
        temp_sent.append(temp_clause2)
    elif sentence[0] == 'or' and sentence[1][0] != 'and' and sentence[2][0] == 'and':
        temp_sent.append('and')
        temp_clause1 = ['or']
        temp_clause1.append(sentence[1])
        temp_clause1.append(sentence[2][1])
        temp_sent.append(temp_clause1)
        temp_clause2 = ['or']
        temp_clause2.append(sentence[1])
        temp_clause2.append(sentence[2][2])
        temp_sent.append(temp_clause2)
    elif sentence[0] == 'or' and sentence[1][0] == 'and' and sentence[2][0] != 'and':
        temp_sent.append('and')
        temp_clause1 = ['or']
        temp_clause1.append(sentence[1][1])
        temp_clause1.append(sentence[2])
        temp_sent.append(temp_clause1)
        temp_clause2 = ['or']
        temp_clause2.append(sentence[1][2])
        temp_clause2.append(sentence[2])
        temp_sent.append(temp_clause2)
    else:
        temp_sent = copy.deepcopy(sentence)
    return temp_sent    
""" Traverse recursively through the list to locate iff """
def checkForIff(sentence):        
    if sentence[0] == 'iff':
        rf_sentence = eliminateIff(sentence)
    else:
        rf_sentence = copy.deepcopy(sentence)
    for clause in range(1,len(rf_sentence)):
        if rf_sentence[clause][0] == 'iff':
            rf_sentence[clause] = checkForIff(rf_sentence[clause])            
        for element in range(1,len(rf_sentence[clause])):
            if rf_sentence[clause][element][0] == 'iff':
                rf_sentence[clause] = checkForIff(rf_sentence[clause])
            rf_sentence[clause][element] = checkForIff(rf_sentence[clause][element])
    return rf_sentence

""" Traverse recursively through the list to locate implies """
def checkForImplies(sentence):
    if sentence[0] == 'implies':
        rf_sentence = eliminateImplication(sentence)
    else:
        rf_sentence = copy.deepcopy(sentence)
    for clause in range(1,len(rf_sentence)):
        if rf_sentence[clause][0] == 'implies':
            rf_sentence[clause] = checkForImplies(rf_sentence[clause])            
        for element in range(1,len(rf_sentence[clause])):
            if rf_sentence[clause][element][0] == 'implies':
                rf_sentence[clause] = checkForImplies(rf_sentence[clause])
            rf_sentence[clause][element] = checkForImplies(rf_sentence[clause][element])
    return rf_sentence

""" Traverse recursively through the list to locate nots next to each other """
def checkForNotDNeg(sentence, found, first):
    firstAsNot = 0
    rf_sentence = copy.deepcopy(sentence)
    if rf_sentence[0] == 'not':
        if first == 0:
            if found == 1:
                rf_sentence = applyDoubleNegetion(sentence)
        else:
            firstAsNot = 1
        found = 1
    else:
        found = 0
    first = 0
    for clause in range(1,len(rf_sentence)):
        if rf_sentence[clause][0] == 'not':
            if found == 1:
                if firstAsNot == 1:
                    rf_sentence = applyDoubleNegetion(rf_sentence)
                    rf_sentence[clause] = checkForNotDNeg(rf_sentence[clause], found, first)
                else:
                    found = 0
                    rf_sentence[clause] = checkForNotDNeg(rf_sentence[clause], found, first)
                found = 0
            else:
                found = 1
                rf_sentence[clause] = copy.deepcopy(rf_sentence[clause])              
        else:
            found = 0            
        for element in range(1,len(rf_sentence[clause])):
            if rf_sentence[clause][element][0] == 'not':
                if found == 1:
                    rf_sentence[clause] = checkForNotDNeg(rf_sentence[clause], found, first)
                    found = 0
                else:
                    found = 1
            else:
                found = 0
                rf_sentence[clause][element] = checkForNotDNeg(rf_sentence[clause][element], found, first)
    return rf_sentence

""" Traverse recursively through the list to locate not to apply DeMorgans"""
def checkForNotDMorgan(sentence):
    if sentence[0] == 'not':
        rf_sentence = applyDeMorgans(sentence)
        found = 0;first = 1
        rf_sentence = checkForNotDNeg(rf_sentence, found, first)
    else:
        rf_sentence = copy.deepcopy(sentence)
    for clause in range(1,len(rf_sentence)):
        if rf_sentence[clause][0] == 'not':
            rf_sentence[clause] = checkForNotDMorgan(rf_sentence[clause])    
        for element in range(1,len(rf_sentence[clause])):
            if rf_sentence[clause][element][0] == 'not':
                rf_sentence[clause] = checkForNotDMorgan(rf_sentence[clause])
            else:
                rf_sentence[clause][element] = checkForNotDMorgan(rf_sentence[clause][element])
    return rf_sentence
""" Traverse recursively through the list to locate or to apply Distributive laws """
def checkForOr(sentence):
    if sentence[0] == 'or':
        rf_sentence = applyDistributive(sentence)
    else:
        rf_sentence = copy.deepcopy(sentence)
    for clause in range(1,len(rf_sentence)):
        if rf_sentence[clause][0] == 'or':
            rf_sentence[clause] = checkForOr(rf_sentence[clause])            
        for element in range(1,len(rf_sentence[clause])):
            if rf_sentence[clause][element][0] == 'or':
                rf_sentence[clause] = checkForOr(rf_sentence[clause])
            rf_sentence[clause][element] = checkForOr(rf_sentence[clause][element])
    return rf_sentence

""" Combine adjacent ands"""
def combineAnd(sentence):
    new_sent = ['and']
    for clause in range(1,len(sentence)):
        if sentence[clause][0] == 'and':
            for literals in range(1,len(sentence[clause])):
                new_sent.append(sentence[clause][literals])
        else:
            new_sent.append(sentence[clause])
    return new_sent

""" Traverse recursively through the list to locate and, combine after that"""
def joinAllAnd(sentence):
    if sentence[0] == 'and':
        rf_sentence = combineAnd(sentence)
    else:
        rf_sentence = copy.deepcopy(sentence)
    for clause in range(1,len(rf_sentence)):
        if rf_sentence[clause][0] == 'and':
            rf_sentence[clause] = joinAllAnd(rf_sentence[clause])            
        for element in range(1,len(rf_sentence[clause])):
            if rf_sentence[clause][element][0] == 'and':
                rf_sentence[clause] = joinAllAnd(rf_sentence[clause])
            rf_sentence[clause][element] = joinAllAnd(rf_sentence[clause][element])
    return rf_sentence

""" Combine neighbouring or """
def combineOr(sentence):
    new_sent = ['or']
    for clause in range(1,len(sentence)):
        if sentence[clause][0] == 'or':
            for literals in range(1,len(sentence[clause])):
                new_sent.append(sentence[clause][literals])
        else:
            new_sent.append(sentence[clause])
    return new_sent

""" Traverse recursively through the list to locate or"""
def joinAllOr(sentence):
    if sentence[0] == 'or':
        rf_sentence = combineOr(sentence)
    else:
        rf_sentence = copy.deepcopy(sentence)
    for clause in range(1,len(rf_sentence)):
        if rf_sentence[clause][0] == 'or':
            rf_sentence[clause] = joinAllOr(rf_sentence[clause])            
        for element in range(1,len(rf_sentence[clause])):
            if rf_sentence[clause][element][0] == 'or':
                rf_sentence[clause] = joinAllOr(rf_sentence[clause])
            rf_sentence[clause][element] = joinAllOr(rf_sentence[clause][element])
    return rf_sentence

""" Removes duplicate literals and duplicate clauses"""
def eliminateDuplicates(sentence):
    if sentence[0] == 'and':
        checked = []
        sortedl = []
        checked.append('and')
        sortedl.append('and')
        for clause in range(1,len(sentence)):
            if sentence[clause][0] == 'or':
                checked_element = ['or']
                for element in range(1,len(sentence[clause])):
                    if sentence[clause][element] not in checked_element:
                        checked_element.append(sentence[clause][element])
                checked.append(checked_element)
                sortedl.append(sorted(checked_element))
            else:
                checked.append(sentence[clause])
                sortedl.append(sorted(sentence[clause]))
    else:
        checked = copy.deepcopy(sentence)
        sortedl = copy.deepcopy(sorted(sentence))  
    temp_sent = []
    temp_sent_s = []
    if checked[0] == 'and':
        temp_sent.append('and')
        temp_sent_s.append('and')
        for clause in range(1,len(checked)):
            if checked[clause] not in temp_sent:
                if sorted(checked[clause]) not in temp_sent_s:
                    temp_sent.append(checked[clause])
                    temp_sent_s.append(sorted(checked[clause]))
    else:
        temp_sent = copy.deepcopy(checked)
    if temp_sent[0] == 'and' and len(temp_sent) == 2:
        if temp_sent[1][0] != 'or':
            return "'" + temp_sent[1] + "'"
        else:
            return temp_sent[1]
    else:
        return temp_sent
    
""" Eliminate each of the operators to the given propositional logic to convert into cnf"""
def processLogic(sentence):
    exp_sent = checkForIff(sentence)
    exp_sent = checkForImplies(exp_sent)
    exp_sent = checkForNotDMorgan(exp_sent)
    exp_sent = checkForOr(exp_sent)
    exp_sent = checkForOr(exp_sent)
    exp_sent = joinAllOr(exp_sent)
    exp_sent = joinAllAnd(exp_sent)
    exp_sent = eliminateDuplicates(exp_sent)
    return exp_sent

""" Reads input from the file - line by line"""
for instr in inFile:
    sentence = eval(instr)
    if instr[0] == '[':
        result = processLogic(sentence)
        print result
        outfile.write("%s\n" %result)
inFile.close()
outfile.close()
