#!/bin/bash

echo "Setting up MiniLang Pattern Generator..."

# Compile the compiler
echo "Compiling MiniLang compiler..."
g++ minilang.cpp -o minilang -std=c++17

if [ $? -ne 0 ]; then
    echo "Error: Failed to compile MiniLang compiler!"
    exit 1
fi

# Create all pattern snippets
echo "Creating pattern snippets..."

# Factorial
cat > factorial.minilang << 'EOF'
n = 5;
res = 1;
i = 1;
while (i <= n) {
  res = res * i;
  i = i + 1;
}
print(res);
EOF

# Primes
cat > primes.minilang << 'EOF'
n = 10;
count = 0;
num = 2;
while (count < n) {
    isPrime = 1;
    i = 2;
    while (i * i <= num + 1) {
        if (num % i == 0) {
            isPrime = 0;
        }
        i = i + 1;
    }
    if (isPrime == 1) {
        print(num);
        count = count + 1;
    }
    num = num + 1;
}
EOF

# Arithmetic
cat > arithmetic.minilang << 'EOF'
a = 5;
d = 3;
n = 8;
i = 0;
while (i < n) {
    term = a + i * d;
    print(term);
    i = i + 1;
}
EOF

# Geometric
cat > geometric.minilang << 'EOF'
a = 2;
r = 3;
n = 6;
i = 0;
while (i < n) {
    term = a;
    j = 0;
    while (j < i) {
        term = term * r;
        j = j + 1;
    }
    print(term);
    i = i + 1;
}
EOF

# Triangular
cat > triangular.minilang << 'EOF'
n = 7;
i = 1;
while (i <= n) {
    triangular = i * (i + 1) / 2;
    print(triangular);
    i = i + 1;
}
EOF

# Compile menu
echo "Compiling menu system..."
g++ menu.cpp -o menu -std=c++17

if [ $? -ne 0 ]; then
    echo "Error: Failed to compile menu system!"
    exit 1
fi

echo "Setup complete!"
echo "Run './menu' to start the MiniLang Pattern Generator"
