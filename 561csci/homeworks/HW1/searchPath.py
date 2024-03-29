import sys
from collections import defaultdict

""" READ THE INPUT FILE """
inputFile = open(sys.argv[2])
conf = 0
width = 0
height = 0
start = [0,0]
target = [0,0]
config = defaultdict(lambda: defaultdict(int))
row = 0
for line in inputFile:
    if conf==0:
        spl = line.strip().split(' ')
        if spl[0]=='WIDTH':
            width = int(spl[1])
        elif spl[0]=='HEIGHT':
            height = int(spl[1])
        elif spl[0]=='agent':
            start[0] = int(spl[1].split(',')[0])
            start[1] = int(spl[1].split(',')[1])
        elif spl[0]=='target':
            target[0] = int(spl[1].split(',')[0])
            target[1] = int(spl[1].split(',')[1])
        elif spl[0]=='configuration':
            conf = 1
        else:
            print 'Error reading input file.'
            sys.exit()
    else:
        spl = [int(x) for x in line.split('\t')]
        for column in range(width):
            config[row][column] = spl[column]
        row += 1
inputFile.close()

""" Check for Valid START and TARGET """
if start[0]<0 or start[0]>=height or start[1]<0 or start[1]>=width:
    print 'Invalid START position'
    sys.exit(0)

if target[0]<0 or target[0]>=height or target[1]<0 or target[1]>=width:
    print 'Invalid TARGET position'
    sys.exit(0)

""" Breath First Search """
def BFS(width, height, start, target, config):
    found = 0
    frontier = []
    explored = []
    parent = defaultdict(list)
    frontier.append(start)
    while len(frontier)>0:
        node = frontier.pop(0)
        explored.append(node)
        if node[0]>0:
            # Go UP
            child = [node[0]-1,node[1]]
            childHash = float(str(child[0])+'.'+str(child[1]))
            if child not in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                if child==target:
                    # Target found
                    found = 1
                    parent[childHash] = node
                    break
                frontier.append(child)
                parent[childHash] = node
        if node[0]<height-1:
            # Go DOWN
            child = [node[0]+1,node[1]]
            childHash = float(str(child[0])+'.'+str(child[1]))
            if child not in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                if child==target:
                    # Target found
                    found = 1
                    parent[childHash] = node
                    break
                frontier.append(child)
                parent[childHash] = node
        if node[1]>0:
            # Go LEFT
            child = [node[0],node[1]-1]
            childHash = float(str(child[0])+'.'+str(child[1]))
            if child not in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                if child==target:
                    # Target found
                    found = 1
                    parent[childHash] = node
                    break
                frontier.append(child)
                parent[childHash] = node
        if node[1]<width-1:
            # Go RIGHT
            child = [node[0],node[1]+1]
            childHash = float(str(child[0])+'.'+str(child[1]))
            if child not in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                if child==target:
                    # Target found
                    found = 1
                    parent[childHash] = node
                    break
                frontier.append(child)
                parent[childHash] = node
    if found==0:
        return -1
    path = []
    cur = target
    while cur!=start:
        path.append(cur)
        curHash = float(str(cur[0])+'.'+str(cur[1]))
        cur = parent[curHash]
    path.append(start)
    path = path[::-1]
    return path

""" Uniform Cost Search """
def UCS(width, height, start, target, config):
    found = 0
    """Newly added code"""
    tt_cost = defaultdict(lambda: defaultdict(int))
    """Newly added code ends"""
    frontier = []
    frontierCost = []
    explored = []
    parent = defaultdict(list)
    frontier.append(start)
    frontierCost.append(config[start[0]][start[1]])
    while len(frontier)>0:
        node = frontier.pop(0)
        """Newly added code"""
        frontierCost.pop(0)
        """Newly added code ends"""
        explored.append(node)
        if node[0]>0:
            # Go UP
            child = [node[0]-1,node[1]]
            childHash = float(str(child[0])+'.'+str(child[1]))
            if child not in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                if child==target:
                    # Target found
                    found = 1
                    parent[childHash] = node
                    """Newly modified code"""
                    """break"""
                    """Newly modified code ends"""
                frontier.append(child)
                """Newly modified code"""
                if node!=start:
                    print "parent cost: ", config[node[0]][node[1]]
                    tt_cost[child[0]][child[1]] = tt_cost[node[0]][node[1]] + config[child[0]][child[1]]
                    "frontierCost.append(config[child[0]][child[1]] + tt_cost[node[0]][node[1]])"
                    frontierCost.append(tt_cost[child[0]][child[1]])
                else:
                    frontierCost.append(config[child[0]][child[1]])
                    tt_cost[child[0]][child[1]] = config[child[0]][child[1]]
                parent[childHash] = node
            elif child in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                temp_cost = tt_cost[node[0]][node[1]] + config[child[0]][child[1]]
                if temp_cost < tt_cost[child[0]][child[1]]:
                    tt_cost[child[0]][child[1]] = temp_cost
                    temp_index = frontier.index(child)
                    frontierCost[temp_index] = temp_cost
                    parent[childHash] = node
                    """Newly modified code ends"""
        if node[0]<height-1:
            # Go DOWN
            child = [node[0]+1,node[1]]
            childHash = float(str(child[0])+'.'+str(child[1]))
            if child not in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                if child==target:
                    # Target found
                    found = 1
                    parent[childHash] = node
                    """break"""
                frontier.append(child)
                """Newly modified code"""
                if node!=start:
                    print "parent cost: ", config[node[0]][node[1]]
                    tt_cost[child[0]][child[1]] = tt_cost[node[0]][node[1]] + config[child[0]][child[1]]
                    "frontierCost.append(config[child[0]][child[1]] + tt_cost[node[0]][node[1]])"
                    frontierCost.append(tt_cost[child[0]][child[1]])
                else:
                    frontierCost.append(config[child[0]][child[1]])
                    tt_cost[child[0]][child[1]] = config[child[0]][child[1]]
                parent[childHash] = node
            elif child in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                temp_cost = tt_cost[node[0]][node[1]] + config[child[0]][child[1]]
                if temp_cost < tt_cost[child[0]][child[1]]:
                    tt_cost[child[0]][child[1]] = temp_cost
                    temp_index = frontier.index(child)
                    frontierCost[temp_index] = temp_cost
                    parent[childHash] = node
                """Newly modified code ends"""    
        if node[1]>0:
            # Go LEFT
            child = [node[0],node[1]-1]
            childHash = float(str(child[0])+'.'+str(child[1]))
            if child not in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                if child==target:
                    # Target found
                    found = 1
                    parent[childHash] = node
                    """break"""
                frontier.append(child)
                """Newly modified code"""
                if node!=start:
                    print "parent cost: ", config[node[0]][node[1]]
                    tt_cost[child[0]][child[1]] = tt_cost[node[0]][node[1]] + config[child[0]][child[1]]
                    "frontierCost.append(config[child[0]][child[1]] + tt_cost[node[0]][node[1]])"
                    frontierCost.append(tt_cost[child[0]][child[1]])
                else:
                    frontierCost.append(config[child[0]][child[1]])
                    tt_cost[child[0]][child[1]] = config[child[0]][child[1]]
                parent[childHash] = node
            elif child in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                temp_cost = tt_cost[node[0]][node[1]] + config[child[0]][child[1]]
                if temp_cost < tt_cost[child[0]][child[1]]:
                    tt_cost[child[0]][child[1]] = temp_cost
                    temp_index = frontier.index(child)
                    frontierCost[temp_index] = temp_cost
                    parent[childHash] = node
                """Newly modified code ends"""
        if node[1]<width-1:
            # Go RIGHT
            child = [node[0],node[1]+1]
            childHash = float(str(child[0])+'.'+str(child[1]))
            if child not in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                if child==target:
                    # Target found
                    found = 1
                    parent[childHash] = node
                    """break"""
                frontier.append(child)
                """Newly modified code"""
                if node!=start:
                    print "parent cost: ", config[node[0]][node[1]]
                    tt_cost[child[0]][child[1]] = tt_cost[node[0]][node[1]] + config[child[0]][child[1]]
                    "frontierCost.append(config[child[0]][child[1]] + tt_cost[node[0]][node[1]])"
                    frontierCost.append(tt_cost[child[0]][child[1]])
                else:
                    frontierCost.append(config[child[0]][child[1]])
                    tt_cost[child[0]][child[1]] = config[child[0]][child[1]]
                parent[childHash] = node
            elif child in frontier and child not in explored and config[child[0]][child[1]]!=-1:
                temp_cost = tt_cost[node[0]][node[1]] + config[child[0]][child[1]]
                if temp_cost < tt_cost[child[0]][child[1]]:
                    tt_cost[child[0]][child[1]] = temp_cost
                    temp_index = frontier.index(child)
                    frontierCost[temp_index] = temp_cost
                    parent[childHash] = node
                """Newly modified code ends"""
        """Newly added code"""
        "[frontier_s for (frontierCost_s,frontier_s) in sorted(zip(frontierCost,frontier), key=lambda pair: pair[0])]"
        "[frontier_s for (frontierCost_s,frontier_s) in sorted(zip(frontierCost,frontier))]"
        "frontier_s, frontierCost_s = zip(*sorted(zip(frontierCost,frontier)))"
        "print 'FCO:', frontierCost"
        "print 'FO: ', frontier"
        frontier = [x for (y,x) in sorted(zip(frontierCost,frontier))]
        "frontier = frontier_s"
        print 'FOl:', frontier
        "print 'FN: ', frontier_s"
        frontierCost.sort(reverse=False)
        print 'FCN:', frontierCost
        "print 'FCS:', y"
        """Newly added code ends"""
    if found==0:
        return -1
    path = []
    cur = target
    while cur!=start:
        path.append(cur)
        curHash = float(str(cur[0])+'.'+str(cur[1]))
        cur = parent[curHash]
    path.append(start)
    path = path[::-1]
    return path

""" READ SYSTEM INPUT TO DETERMINE SEARCH STRATEGY """
algorithm = int(sys.argv[4])
if algorithm not in [1,2]:
    print 'Wrong input for algorithm'
    sys.exit()

if algorithm==1:
    """ Breath First Search """
    path = BFS(width, height, start, target, config)
    if path==-1:
        print "No path exists"
        sys.exit()
else:
    """ Uniform Cost Search """
    path = UCS(width, height, start, target, config)
    if path==-1:
        print "No path exists"
        sys.exit()

""" PRINT THE PATH """
print ''
s = ' '
for i in range(width+1):
    if i==0:
        s += ' '
        continue
    s += str(i-1)
print s

pathCost = 0
for i in range(height):
    s = ''
    for j in range(width+1):
        if j==0:
            s += str(i)+str(' ')
            continue
        if [i,j-1]==start:
            s += 'A'
        elif [i,j-1]==target:
            s += 'T'
            pathCost += config[target[0]][target[1]]
        elif [i,j-1] in path:
            s += '.'
            pathCost += config[i][j-1]
        elif config[i][j-1]==-1:
            s += 'X'
        else:
            s += ' '
    print s
print ''
print "Path cost:", pathCost
