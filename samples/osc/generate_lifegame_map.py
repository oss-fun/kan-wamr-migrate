import random

def zerorize(num):
    print('{', end='')
    for j in range(num):
        print('0', end='')
        if (j < num-1):
            print(', ', end='')
    print('},')


zerorize(22)
for i in range(20):
    print('{', end='')
    for j in range(22):
        if (j == 0):
            print(0, end=', ')
        elif (j == 21):
            print(0, end='')
        else:
            print(random.randint(0, 100) % 2, end=', ')
    print('},')
zerorize(22)
