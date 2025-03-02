-- x = 1, y = 1
lia 1  
psh
psh

loop:
    --load a, b = x, y
    pop -- a = y
    swp -- b = y
    pop -- a, b = x, y
    
    --calculate z
    add -- a, b = z, y

    --push y then z onto stack (so x = y, y = z)
    swp -- a, b = y, z
    psh -- stack x = y
    swp -- a, b = z, y
    psh -- stack y = z


    --stop at 28657, the 23rd fibonacci number
    lib 28657
    teq
    jez loop
    hlt