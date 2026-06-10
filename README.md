# Pharmacy Stock Management System

A comprehensive C application for managing pharmacy medication inventory, suppliers, and stock operations.

## Features
- Add and manage medications
- Track suppliers and batch information
- Place orders and manage stock levels
- Persistent data storage
- User-friendly menu interface

## Building and Running
```bash
gcc -Wall -Wextra -std=c11 -o pharmacy main.c
./pharmacy
```

The program automatically loads data from `TxtMed.txt` at startup and can save stock changes back to this file from the menu.
