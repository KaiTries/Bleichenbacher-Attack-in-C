This is an implementation of the Bleichenbacher Attack in the C language. It utilizes the arbitrarily large integers by gmp. It focuses on the implementation of the algorithm. So N, d and e are hardcoded, and the padding is just repeating 1s.

I built it on mac so so all I had to do was 
```bash
brew install gmp
```

To run compile the script simply run
```bash
cc bleichenbacher_base.c -I /opt/homebrew/include -L /opt/homebrew/lib -lgmp
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

After compiling simpliy run
```bash
./a.out
```