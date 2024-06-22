# Implementation of Bleichenbacher Attack and select optimizations in C
This repo implements the bleichenbacher attack as described in the original paper, as well as some additional improvements. It is written fully in C and utilizes the gmp library for the large integers.
## Results
Test result have been obtained by letting the attack run 100 times for each configuration. Shown are the Mean and Median Values.

|                             |  Base Attack  |   Trimming    |       Improved Step Two     | Fully optimized |
|-----------------------------|---------------|---------------|-----------------------------|-----------------|
| Average Oracle Queries      | 21355 / 10704 | 21571 / 10805 |          19727 / 9447       |   15645 / 5649  |
| Average queries for step2a  |  8545 / 2618  |  8004 / 2398  |           6916 / 2244       |    2078 / 104   |
| Average run time            | 4.97s / 1.26s |  4.9s / 2.2s  |          4.59s / 1.13s      |   3.5s / 1.33s  |


Below is the table for the results achieved by modifying the original python implementation to mirror the improvements.

|                             |   Base Attack  |   Trimming   |       Improved Step Two     | Fully optimized |
|-----------------------------|----------------|--------------|-----------------------------|-----------------|
| Average Oracle Queries      |  45350 / 12106 | 38515 / 4072 |          42010 / 12106      |   7049 / 3240   |
| Average queries for step2a  |  40021 / 8295  | 33197 / 158  |          36681 / 8295       |   1730 / 8      |
| Average run time            |  10.8s / 2.89s | 9.2s / 0.87s |          10.6s / 2.84s      |   1.6s / 0.68s  |

- On line 221 and 222 we can set the values of a and b to B2 and B3 - 1 to undo the trimming
- I added an additional function "step2Base" that can be used to swap out the improvements

As one can see the results have large differences. Except for the average amount of queries for the fully optimized attack all the results differ strongly.


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
void findNextS_iteratively() {
    while(1) {
        // compute c_prime
        if (oracle(c_prime)) {
            // found new valid s
        }
        // increase s by one
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
## Improvements
Since the invention of the original algorithm several improvements have been suggested. [this paper](https://www.royalholloway.ac.uk/media/9093/techreport-gageboyle.pdf) highlights some major improvements over the past 20 years. It also serves as one of the papers my implementations are based on, the other one being [this article](https://link.springer.com/chapter/10.1007/978-3-031-17510-7_10).

### Trimmers
One of the main findings was that just as multiplication and exponantion can be used to find the original message, division is just as useful in making the search space smaller. So the first improvement that I implemented was the trimmers. Trimming is to be done before the start of the algorithm, but an oracle is still needed for this step. The idea is, that there exist some integer t that divides the original message m and using this integer we can make the initial search space smaller than starting betwen 2B and 3B.

The relevant code for this improvement can be found [here](https://github.com/KaiTries/Bleichenbacher-Attack-in-C/blob/main/bleichenbacher.c#L103). The reason for the specific limiters of t and the amount of queries, is because the original authors of the method were a bit obscure and the author of the current paper found these values through stochastics. There is no actual scientific proof, but it was found, that 500 oracle calls are the most optimal amount and that the values for t/u should be between 2/3 and 3/2. Using binary search to find the lower and upper u values is crucial, since the search space can get rather big.

```c
void trimming() {
    // initialize data structures to hold candidates for u and t
    
    // loop through possible t values
    for (int t = 3; t < 10000; t++) {
        // u and t should be close together so real values are t-1 / t and t+1 / t
        for (u = t-1; u < t+2; u++) {
            // if t and u are valid ad them to the data structures
            if (u & t valid) {
                us[idx] = u;
                ts[idx++] = t;
            }
        }
    }
    

    // it is possible (but unlikely) that no trimmers are found, set to 1 in this case -> 2B and 3B
    if (idx == 0) {
        // set outputs to 1
    }

    // for t find the lcm of all found t values
    int denom = lcm(ts,idx);

    // 2t / 3 < u < 3t / 2

    // max bound for u_lower is u_a / t_a
    // min bound for u_upper is u_b / t_b


    // binary search for upper and lower u
    while(max_u_lower - min_u != 1) {
        // search
    }

    while(min_u_upper + 1 !=  max_u) {
        // search
    }
    // return the two found us
}
```
### Improvement for searching over multiple intervals 
Another proposed improvement is to step 2b, which describes the procedure when there are multiple intervals left in the set of intervals. Since in the original interval this just meant iteratively searching for a new integer s. As iteratively searching for a new s is very slow, the proposed change is to use the method of step 2c and adapt it to multiple intervals. This adaption make sense, since we know that the original m has to be in any of the intervals, so the advanced search will work. 

In my Implementation, I simply iterated over the set of intervals and applied step2c to one interval after the other, stopping once i found a new valid s. This implementation could be further improved, by paralellising it and stopping all threads once one has found a solution.
```c
void findNextS_multipleIntervals() {
    // iterate over set of intervals
    for (size_t j = 0; j < set->size; j++) {
        // do step 3c for each interval
        int result = searchingWithOneIntervalLeft(&set->intervals[j], c, s);
        // abandon the left over intervals once done
        if(result) break;
    }
}
```
### Better way to search for s instead of iteratively
In the original algorithm a lot of time is spent finding the first s in step 2a. Since we just iteratively increase s by 1 and check if it is valid. We can improve this step by skipping over s values that cannot produce a valid result and by increasing the starting value. Decreasing the lower bound (increasing the starting value) has the biggest effect, if it is done in combination with the trimming. Below it is demonstrated, that the first s must be larger or equal to $\frac{n + 2B}{3B - 1}$. 
$\\
m_0 \cdot s_1 \geq n + 2B \\
(3B - 1) \cdot s_1 \geq n + 2B \\
s_1 \geq \frac{n + 2B}{3B - 1}\\
$
Since in the trimming step we replace the initial higher bound of 3B - 1 with b we end up with $s_1 \geq \frac{n + 2B}{b}$. Which gives us a better starting point. The second way to improve step 2a is to skip values of s that cannot produce valid results. This is doable, because even if we do not know yet which s will be correct, we know that if we find a correct s $2B \leq m_0 \cdot s_1 \mod n < 3B$ will hold. This knowledge allows us to rearrange the equation to: $\frac{2B + jn}{b} \leq s_1 < \frac{3B + jn}{a}$. As a result from this, we know that for any given integer j for which the following is true: $\frac{3B + jn}{a} < \frac{2B + (j+1)n}{b}$. No suitable s in this range can be found! Therefore we can skip all values $\frac{3B + jn}{a} \leq s_1 < \frac{2B + (j+1)n}{b}$.

My implementation of this improvement can be found [here](https://github.com/KaiTries/Bleichenbacher-Attack-in-C/blob/main/bleichenbacher.c#L193). It follows the original implementation by the authors. Since we are given the initial value of s, we rearrange the equation of $\frac{3B + jn}{a} \leq s$ to $\frac{(s * a) - 3B}{n} = j$. This gives us a j for which we can then calculate the second point $\frac{2B + (j + 1) * n}{b}$. If this second point is bigger than the first we know that the s value cannot be valid and we will not query the oracle.
```c
void findNextS_2a() {
    // s is initially equal to (n + 2B) / b
    while(1) {
        // j = ((s * a) - 3B) / n
        // compare = (2B + (j + 1) * n) / b

        // check if s is in the hole given by j and compare
        if (mpz_cmp(*s,comparison) >= 0) {
            // compute c_prime and query oracle
            if (oracle(&c_prime, &rsa)) {
                // if oracle returns 1 we found next valid s
                return;
            }
            // increase s by one and repeat
            mpz_add_ui(*s,*s,1);
        } else {
            // if s is in the hole we set s to the end of the hole and repeat
            mpz_set(*s, comparison);
        }
    }
}
```
## How to use
Simply clone the repository and follow the steps below to try it out yourself!

### Dependencies
The only non standard dependency that you might need to additionally install is the gmp library. On mac gmp can be install with brew as seen below. Otherwise it can also be downloaded from [here](https://gmplib.org/#DOWNLOAD).

```bash
brew install gmp
```

To build the script simply run (make sure you have the correct folders linked):
```bash
make
```

After building you can execute the script with:
```bash
./a.out
```
