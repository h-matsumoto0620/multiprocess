#!/usr/bin/ env python
#coding: utf-8

import random

data_num = 10000

foods = ["potate", "chicken", "onion", "cumin", "turmeric", "cardamom", "coriander", "ginger", "red_pepper"]

val_min = 1
val_max = 999

for food in foods:

    f = open(food+".txt","w")

    for i in xrange(data_num):
        value = random.randint(val_min, val_max)

        p = random.randint(0, 10000)

        if p <= 0 and p < 1:
            state = 1
        elif 1 <= p and p < 2500:
            state = 2
        elif 2500 <= p and p < 7500:
            state = 3
        elif 7500 <= p and p < 9000:
            state = 4
        elif 9000 <= p and p < 10000:
            state = 5

        f.write("%d %d\n" %(value, state))

    f.close()
