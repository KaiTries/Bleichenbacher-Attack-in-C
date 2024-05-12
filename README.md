This is an implementation of the Bleichenbacher Attack in the C language. It utilizes the arbitrarily large integers by gmp. It focuses on the implementation of the algorithm. So a valid RSA Key pair is hardcoded, and the padding is just repeating 1s.

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