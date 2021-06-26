def tokenize(line):
    x = set()
    n = len(line)
    line = line.lower()
    start = 0
    end = -1
    for i in range(n):
        if line[i].isalnum():
            end = i
        else:
            if end >= start:
                x.add(line[start:end+1])
            start = i + 1
    return x

def hash(x):
    l = len(x)
    sum = 0
    for i in range(l-1):
        sum *= 37
        '''
        sum += ord(x[i])# - ord('0')
        '''
        if x[i].isnumeric():
            sum += int(x[i])
        else:
            sum += ord(x[i]) - ord('a') + 10
        global j
        sum %= j
    return sum

        

if __name__ == "__main__":
    '''
    token = set()
    # names = set()
    for i in range(10000):
        x = open("test_data/mail"+str(i+1), 'r')
        lines = x.readlines()
        # names.add(lines[0][6:])
        # names.add(lines[4][4:])
        token |= tokenize(lines[3][9:])
        token |= tokenize(lines[6])
        x.close()
    y = open("tokens", 'w')
    for i in token:
        if i[-1] != '\n':
            i += '\n'
        y.write(i)
    y.close()
    
    # print(len(names), "names in total")
    '''
    x = open("tokens", 'r')
    b = x.readlines()
    x.close()
    print(len(b), "tokens in total")
    
    global j
    j = 10**9+7 #100030001 23010067 73176001
    aaa = set()
    a = dict()
    collisionh = list()
    for i in b:
        h = hash(i)
        #a[len(i)-2].add(hash(i))
        if h in a:
            aaa.add(i)
            aaa.add(a[h])
            collisionh.append(h)
        else:
            a[h] = i
    print(len(a))
    print(len(aaa))
    print(len(collisionh))

    exit()
    j = 2
    while j <= 10**5:
        #a = list()
        #for i in range(51):
            #a.append(set())
        # print(a)
        kkk = dict()
        for i in aaa:
            hhh = hash(i)
            if hhh in a:
                break
            elif hhh in kkk:
                break
            else:
                kkk[hhh] = i
        else:
            print(j)
            break
        
        j += 1
        #aa = [len(i) for i in a]
        #print(aa)
        '''
        if len(a) == 138078:
            print(j)
            break
        if j % 1000 == 0:
            print("1000 times")
        '''
    # print("collision at these numbers",collisionh)