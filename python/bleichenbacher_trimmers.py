from __future__ import division
import gmpy2
from gmpy2 import *
import math
from math import gcd, ceil, floor
import random
from random import randint
import time
import numpy as np


def eea(a, b):
    if b == 0:
        return (1, 0)
    (q, r) = (a // b, a % b)
    (s, t) = eea(b , r)
    return (t, s - (q * t))

def find_inverse(x, y):
    inv = eea(x, y)[0]
    if inv < 1:
        inv += y
    return inv

def zeroinpaddingcheck(x):
    for j in range(3, 18):
        if j % 2 == 1:
            if hex(x)[j:(j + 2)] == "00":
                return 1
            else:
                continue
        else:
            continue

def zeroinpadding(x):
    for j in range(0, (len(x) - 1)):
        if j % 2 == 0:
            if x[j:(j + 2)] == "00":
                return 1
            else:
                continue
        else:
            continue

def delimiterchecker(x):
    a = len(hex(x))
    b = a - 2

    for j in range(19, (b + 1)):
        if j % 2 == 1:
            if hex(x)[j:(j + 2)] == "00":
                return 1
            else:
                continue
        else:
            continue

def oracle(query):
    global counter
    counter += 1
    v = pow(query,d,n)
    if bottom <= v and v <= top:
        return 1
    return 0

def trimmer_oracle(query):
    global counter2
    counter2 += 1
    v = pow(query, d, n)
    if bottom <= v and v <= top:
        return 1
    return 0

def test1(u, t):
    if gcd(u, t) == 1:
        return 1
    return 0

def test2(u, t):
    tinv = find_inverse(t, n)
    a = pow(u, e, n)
    b = pow(tinv, e, n)
    g = (c_0 * a * b) % n
    if trimmer_oracle(g) == 1:
        return 1
    return 0

def lcm(a):
    lcm = a[0]
    for i in a[1:]:
        lcm = int((lcm * i) / (gcd(lcm, i)))
    return lcm

def trimming():
    global trimmers
    global mintrim
    global maxtrim
    trimmers = [1]
    trimmersfrac = [1]
    trimmer_limit = 500
    for t in range(3, 4097):
        if counter2 < trimmer_limit:
            for u in range((t - 1), (t + 1) + 1):
                if counter2 < trimmer_limit and (u / t) > (2 / 3) and (u / t) < (3 / 2):
                    if test1(u, t) == 1:
                        if test2(u, t) == 1:
                            trimmers.append(u / t)
                            trimmersfrac.append(t)
                            break
                        else:
                            continue
                    else:
                        continue
                else:
                    break
        else:
            break
    denom = lcm(trimmersfrac)
    lowerbottom = floordiv((2 * denom), 3)
    lowertop = denom
    while(lowertop - lowerbottom) != 1:
        u = ceildiv((lowerbottom + lowertop), 2)
        if test2(u, denom) == 1:
            lowertop = u
        else:
            lowerbottom = u
    ulower = lowertop
    mintrim = (ulower / denom)
    upperbottom = denom
    uppertop = ceildiv((3 * denom), 2)
    while(upperbottom + 1) != uppertop:
        u = floordiv((upperbottom + uppertop), 2)
        if test2(u, denom) == 1:
            upperbottom = u
        else:
            uppertop = u
    uupper = upperbottom
    maxtrim = uupper / denom

def range(start, stop):
    while start < stop:
        yield start
        start += 1

def range_overlap_adjsut(list_ranges):
    overlap_corrected = []
    for start, stop in sorted(list_ranges):
        if overlap_corrected and start - 1 <= overlap_corrected[-1][1] and stop >= overlap_corrected[-1][1]:
            overlap_corrected[-1] = (min(overlap_corrected[-1][0], start), stop)
        elif overlap_corrected and start <= overlap_corrected[-1][1] and stop <= overlap_corrected[-1][1]:
            break
        else:
            overlap_corrected.append((start, stop))
    return overlap_corrected

def ceildiv(a, b):
    return -(-a // b)

def floordiv(a, b):
    return a // b

def primes():
    global p
    global q
    global e
    global d
    global k
    global n
    x = random.randint(int(pow(2, 511.5)), int(2 ** 512))
    y = random.randint(int(pow(2, 511.5)), int(2 ** 512))
    p = gmpy2.next_prime(x)
    q = gmpy2.next_prime(y)
    n = p * q 
    byte_length_n = (len(hex(n)) - 2) / 2
    k = int(byte_length_n)
    while k != 128:
        x = random.randint(int(pow(2, 511.5)), int(2 ** 512))
        y = random.randint(int(pow(2, 511.5)), int(2 ** 512))
        p = gmpy2.next_prime(x)
        q = gmpy2.next_prime(y)
        n = p * q 
        byte_length_n = (len(hex(n)) - 2) / 2
        k = int(byte_length_n)
    

    phi = (p - 1) * (q - 1)


    e = 65537
    d = find_inverse(e, phi)

def PMS():
    global message
    a = pow(16, 95)
    b = (pow(16, 96)) - 1
    f = random.randint(a, b)
    message = hex(f)[2: -1]

def pad():
    global padding
    global encoding
    global decimal_of_encoding
    global ciphertext
    r = (k - 3 - ((len(message)) / 2)) * 2
    a = pow(16, (r - 1))
    b = pow(16, r) - 1
    f = random.randint(a, b)
    padding = hex(f)[2: -1]
    while zeroinpadding(padding) == 1:
        g = random.randint(a, b)
        padding = hex(g)[2:-1]
    encoding = "0002" + padding + "00" + message
    decimal_of_encoding = int(encoding, 16)
    ciphertext = pow(decimal_of_encoding, e, n)

def step_1():
    global i
    global c_0
    i = 0
    global a
    global b
    for s in range(1, int(n)):
        w = int(pow(s, e, n))
        binding = int((w * ciphertext) % n)
        if oracle(binding) == 1:
            list_s.append(s)
            break
        else:
            continue
    c_0 = binding
    M = [(2 * B, (3 * B) - 1)]
    trimming()
    a = int(ceil((2 * B) * (1 / mintrim)))
    b = int(floor(((3 * B) - 1) * (1 / maxtrim)))
    M = [(a, b)]
    list_M.append(M)
    i = i + 1

def step_2aBasic():
    global i
    global c_0
    global s
    global counter3
    s = ceildiv((n + (2 * B)), b)
    found = False
    while not found:
        x = int(pow(s, e, n))
        attempt2a = int((x * c_0) % n)
        counter3+=1
        if oracle(attempt2a) == 1:
            list_s.append(int(s))
            found = True
            break
        else:
            s = s + 1
            continue

def step_2a():
    global i
    global c_0
    global s
    global counter3
    s = ceildiv((n + (2 * B)), b)
    found = False
    while not found:
        r = floordiv(((s * a) - (3 * B)), n)
        if s >= ceildiv(((2 * B) + ((r + 1) * n)), b):
            x = int(pow(s, e, n))
            attempt2a = int((x * c_0) % n)
            counter3+=1
            if oracle(attempt2a) == 1:
                list_s.append(int(s))
                found = True
                break
            else:
                s = s + 1
                continue
        else:
            s = ceildiv(((2 * B) + ((r + 1) * n)), b)

def step_2b():
    global i
    global c_0
    global s
    if i > 1 and len(list_M[i - 1]) > 1:
        iteration = 0
        found = False
        r_values = []
        s_ranges = []
        for j in range(0, len(list_M[i - 1])):
            r_values.append(ceildiv(2 * (((list_M[i - 1][j][1] * list_s[i - 1]) - (2 * B))), n))
        for w in range(0, len(list_M[i - 1])):
            s_ranges.append(((ceildiv((2 * B) + (r_values[w] * n), list_M[i - 1][iteration % len(list_M[i - 1])][1])),
                            (ceildiv(((3 * B) + (r_values[w] * n)), list_M[i - 1][iteration % len(list_M[i - 1])][0]))))
        while not found:
            if s_ranges[iteration % len(list_M[i - 1])][0] > s_ranges[iteration % len(list_M[i - 1])][1]:
                h = (r_values[iteration % len(list_M[i - 1])]) + 1
                r_values[iteration % len(list_M[i - 1])] = h
                y = (ceildiv(((2 * B) + (h * n)), list_M[i - 1][iteration % len(list_M[i - 1])][1]), 
                     ceildiv(((3 * B) + (h * n)), list_M[i - 1][iteration % len(list_M[i - 1])][0])) 
                s_ranges[iteration % len(list_M[i - 1])] = y
                s = s_ranges[iteration % len(list_M[i - 1])][0]
                z = int(pow(s, e, n))
                attempt2bnew = int((z * c_0) % n)
                if oracle(attempt2bnew) == 1:
                    found = True
                    list_s.append(int(s))
                    break
                else:
                    t = s + 1
                    s_ranges[iteration % len(list_M[i - 1])] = (t,
                                                                ((ceildiv(((3 * B)+ (r_values[iteration % len(list_M[i - 1])] * n)),
                                                                          list_M[i - 1][iteration % len(list_M[i - 1])][0]))))
                    iteration = iteration + 1
            else:
                s = s_ranges[(iteration % len(list_M[i - 1]))][0]
                z = int(pow(s, e, n))
                attempt2bnew = int((z * c_0) & n)
                if oracle(attempt2bnew) == 1:
                    found = True
                    list_s.append(int(s))
                    break
                else:
                    t = s + 1
                    s_ranges[iteration % len(list_M[i - 1])] = (t,
                                                                ((ceildiv(((3 * B)+ (r_values[iteration % len(list_M[i - 1])] * n)),
                                                                          list_M[i - 1][iteration % len(list_M[i - 1])][0]))))
                    iteration = iteration + 1

def step_2c():
    global i
    global c_0
    global s
    if i > 1 and len(list_M[i - 1]) == 1:
        found = False
        r = ceildiv(2 * (((list_M[i - 1][0][1] * list_s[i - 1]) - (2 * B))), n)
        while not found:
            for s in range(ceildiv(((2 * B) + (r * n)), list_M[i - 1][0][1]),
                           ceildiv(((3 * B) + (r * n)), list_M[i - 1][0][0])):
                z = int(pow(s, e, n))
                attempt2c = int((z * c_0) % n)
                if oracle(attempt2c) == 1:
                    found = True
                    list_s.append(int(s))
                    break
            r = r + 1
    else:
        print("error")

def step_3():
    global i
    global c_0
    list_temp = []

    for j in range(0, len(list_M[i - 1])):
        for r in range((ceildiv(((list_M[i - 1][j][0] * list_s[i]) - (3 * B + 1)), n)),
                       (floordiv((list_M[i - 1][j][1] * list_s[i] - 2 * B), n)) + 1):
            list_temp.append(((int(max(list_M[i - 1][j][0], min(ceildiv((2 * B + r * n), list_s[i]), list_M[i - 1][j][1])))),
                               int(min(list_M[i - 1][j][1], max(floordiv(((3 * B - 1) + r * n), list_s[i]), list_M[i - 1][j][0])))))
    list_M.append(range_overlap_adjsut(list_temp))

def step_4():
    global i
    global c_0
    inverse = find_inverse(list_s[0], n)
    message = list_M[i][0][0] * inverse % n
    if message != decimal_of_encoding:
        print("Error - Decryption failed!")

def main(ciphertext):
    global counter
    global counter2
    global counter3
    global duration
    global bottom
    global top
    global list_s
    global B
    global list_M
    global i
    B = int(pow(2, 8 * (k - 2)))
    bottom = 2 * B
    top = ((3 * B) - 1)
    counter = 0
    counter2 = 0
    counter3 = 0
    list_s = []
    list_M = []
    step_1()
    t0 = time.time()
    step_2a()
    step_3()
    while len(list_M[i]) != 1 or list_M[i][0][0] != list_M[i][0][1]:
        i = i + 1
        if i > 1 and len(list_M[i - 1]) > 1:
            step_2b()
            if t not in list2b:
                list2b.append(t)
        elif i > 1 and len(list_M[i - 1]) == 1:
            step_2c()
        else:
            print("Error")
            break
        step_3()
    else:
        step_4()
    t1 = time.time()
    duration = t1 - t0
    print("it took {} seconds, and required {} calls to the Oracle". format(duration, (counter + counter2)))
    print("It called the oracle for trimming {} times".format(counter2 - 500))

def test(x):
    global list2b
    Oracle_times = []
    trimmercount = []
    oracle2a = []
    times = []
    list2b = []
    u = 1
    for u in range(1, (x + 1)):
        global t
        t = int("90000" + "99" + str(u))
        random.seed(t)
        print("Seed {}: {}".format(u,t))
        primes()
        PMS()
        pad()
        main(ciphertext)
        Oracle_times.append(counter + counter2)
        trimmercount.append(counter2 - 500)
        oracle2a.append(counter3)
        times.append(duration)
        u = u + 1
        print("Overall Oracle Calls - Mean: {}, Median: {}".format(np.mean(Oracle_times), np.median(Oracle_times)))
        print("Step 2a Oracle Calls - Mean: {}, Median: {}".format(np.mean(oracle2a), np.median(oracle2a)))
        print("Overall time taken   - Mean: {}, Median: {}".format(np.mean(times), np.median(times)))
    print("{} seeds have entered step 2b".format(len(list2b)))
    print("The seeds that entered step 2b are {}".format(list2b))

test(500)