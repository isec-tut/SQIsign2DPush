m = 2**131 * 3**78 - 1
R = pow(2,256,m)
R2 = (R*R) % m
words = [(R2 >> (64*i)) & (2**64-1) for i in range(4)]
print('R2_hex=0x{:x}'.format(R2))
for i,w in enumerate(words):
    print('w{} = 0x{:x}'.format(i,w))
