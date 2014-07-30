#!/usr/bin/ env python
#coding: utf-8

import random

foods = ["potate", "chicken", "onion", "cumin", "turmeric", "cardamom", "coriander", "ginger", "red_pepper"]

data_num = 10000
val_min = 100
val_max = 50000


for food in foods:

    f = open(food+".txt","w")

    for i in xrange(data_num):
        value = random.randint(val_min, val_max)

        if random.randint(val_min, val_max) % 257 == 0:
            state = random.randint(1,2)
        else:
            state = random.randint(3,5)

        f.write("%d %d\n" %(value, state))

    f.close()
