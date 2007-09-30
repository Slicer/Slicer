import sys

def readFile(filename):
    # reads a hierarchy file into a python dictionary
    fp = open(filename)

    info = dict()

    for line in fp.readlines():
        line = line.strip()  # strip whitespace
        if line == '' or line[0] == '#':
            # strip empty lines or comments
            continue

        # split the line on the first ":"
        key, value = line.split(':', 1)

        # strip whitespace
        key = key.strip()
        value = value.strip()

        structureName, parameter = key.split('.', 1)

        # look to see if the structure is in the dictionary
        if not info.has_key(structureName):
            info[structureName] = dict()
            
        info[structureName][parameter] = value

    # assign unique numbers:
    uniqueStructureNum = 0
    for structureName in info.keys():
        info[structureName]['id'] = uniqueStructureNum
        uniqueStructureNum += 1

    fp.close()
    return info


if __name__ == '__main__':
    info = readFile(sys.argv[1])

    for structureName in info.keys():
        info[structureName]['parents'] = []

    for structureName in info.keys():        
        sdict = info[structureName]
        if not sdict.has_key('children'): continue
        
        children = sdict['children'].split(',')
        
        for child in children:
            info[child]['parents'].append(structureName)

    aggParents = {}
    for structureName in info.keys():

        parents = info[structureName]['parents']
        if len(parents) != 1:
            print structureName, parents
            for p in parents:
                aggParents[p] = 1

    print "--------"
    for p in aggParents:
        print p, ":", info[p]['children']
