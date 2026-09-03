m = 2**131 * 3**78 - 1
R = pow(2,256,m)
words = [(R >> (64*i)) & (2**64-1) for i in range(4)]
print('m_hex=0x{:x}'.format(m))
print('R_hex=0x{:x}'.format(R))
for i,w in enumerate(words):
    print('w{} = 0x{:x}'.format(i,w))
