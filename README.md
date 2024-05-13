# Implementation of Bleichenbacher Attack and select optimizations in C
This repo implements the bleichenbacher attack as described in the original paper, as well as some additional improvements. It is written fully in C and utilizes the gmp library for the large integers.

## The Attack
The Bleichenbacher attack or also called "million messages attack" was invented in 1998 by Daniel Bleichenbacher. It exploited a weakness in the PKCS #1 v1.5 protocol and is one of the most prominent attacks to date. Specifically the attack exploited the response messages that servers would give upon receiving a wrongly formatted message. The PKCS #1 v1.5 protocol states, that if the message received is incorrectly padded, it should return a distinct error code. This leakage of information was enough for Bleichenbacher to devise an algorithm that given any cyphertext c could find a message m that satisfies $ m = c^{d} \mod n $. The algorithm is roughly dividable into three distinct steps: initial setup & finding s iteratively, finding s with only one interval, and computing new intervals. This will go on until there is only one interval of size 1 left.

### Initial setup & finding s iteratively
What is known:
```
RSA public key: e, N
byte length of modulus N: k
Some ciphertext: c
```

First we set the initial interval (2B,3B - 1) and the starting value of s to n / 3B. Where B is $2^{8*k - 2}$. We then increase s until we obtain our first valid integer s that will result in a valid message:

```c
void findNextS_iteratively(
    mpz_t *c, mpz_t *s, 
    mpz_t *a, mpz_t *b) 
{
    mpz_t c_prime;
    mpz_add_ui(*s,*s,1);
    mpz_init(c_prime);

    while(1) {
        mpz_powm(c_prime, *s, rsa.E, rsa.N);
        mpz_mul(c_prime, c_prime, *c);
        mpz_mod(c_prime, c_prime, rsa.N);
        if (oracle(&c_prime, &rsa)) {
            mpz_clear(c_prime);
            return;
        }
        mpz_add_ui(*s,*s,1);
    }
}
```
This step of finding the next bigger s is also done when there are more than one intervals in out set of intervals. It is a very inefficient part of the algorithm and ways to increase its efficiency exist.
### finding s if only one interval is left
In this case we have sufficient information to search for the next s in a way that is more efficient than just by iteration.
```c
void searchingWithOneIntervalLeft() {   
    // r = (2(bs - 2B)) / n
    while(1) {
        // low_bound = ceil((B2 + r * N), b)
        // high_bound = ceil((B3 - 1 + r * N),a) + 1
        for(s = low_bound; s < high_bound; s++) {
            // query the oracle until next s found
        }
        // increase r by one and repeat
    }
}
```
### Update the set of intervals
Whenever we have found a new s we will update our set of intervals, so that it only contains the possible numbers that are still valid.
```c
void findNewIntervals() {
    // loop through all intervals in priorSet
    for (size_t j = 0; j < priorSet->size; j++) {
        // r1 = ceil((a * s - B3 + 1), N)
        // r2 = floor((b * s - B2), N) + 1
        
        for (r = r1; i <= r2; r++) {
            // aa = ceil(B2 + r*N, s)
            // bb = floor(B3 - 1 + r*N, s)

            // a = max(a,aa)
            // b = min(b,bb)

            if (a < b) {
                // add the interval
            }
        }
    }
    // replace priorSet with newSet
}

```




On mac gmp can be install with brew as seen below. It can also be downloaded from [here](https://gmplib.org/#DOWNLOAD).

```bash
brew install gmp
```

To build the script simply run:
```bash
cc main.c interval.c bleichenbacher_base.c rsa.c -I /opt/homebrew/include -L /opt/homebrew/lib -lgmp
```
You might have to adjust the paths to the include and library folders.
My VSC settings are as follows:
```json
{
    "configurations": [
        {
            "name": "Mac",
            "includePath": [
                "${workspaceFolder}/**",
                "/opt/homebrew/include/**", // if you use brew
                "/opt/homebrew/lib/**"  // if you use brew
            ],
            "defines": [],
            "macFrameworkPath": [
                "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks"
            ],
            "compilerPath": "/usr/bin/clang",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "macos-clang-arm64"
        }
    ],
    "version": 4
}
```

After building you can execute the script with:
```bash
./a.out
```


## rsa.h & rsa.c
Defines a simple RSA struct that holds the public key E, private key D and modulus N. Generate() initializes the given RSA struct.
Currently the P and Q values are hardcoded in the implementation. N and D are then derived from the two primes. The function also 
performs some basic checks that should guarantee that the resulting RSA key is valid.

Also defines the encryption and decryption methods. The methods are just wrappers around the powm function of gmp. call the methods 
with the pointers to the wanted numbers.

The Oracle and the supporting mpz to hex array function are also defined in this file. The oracle first transforms the given number into a char array and then checks if the first two bytes are 00 and 02. It also checks if there is a 00 byte some time after and if it occures atleast 8 bytes after. Returns 1 if successful 0 otherwise.


## interal.h & interval.c
Defines an interval and a set of intervals. The intervals are inclusive of both endpoints. They hold two mpz_t structs. Intervals can be initialized both with ints and mpz_t structs. 

The set is ordered and performs unions upon adding of new intervals.

## bleichenbacher.h
Defines the steps of the attack itself.

## main.h
Performs a bleichenbacher attack with the given user input. 