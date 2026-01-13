# Print Spooler Management System 🖨️

## Course
Data Structures & Algorithms – Final Project  
Semester: Fall 2025

## Project Overview
This project implements a **Print Spooler Management System**
using core **Data Structures & Algorithms** in C++.

The system manages print jobs using priority scheduling,
supports user authentication, and simulates multiple printers.

---

## Key Features
- User login & signup system
- Priority-based print scheduling (Min Heap)
- Fast job lookup using Hash Map
- Job cancellation & status tracking
- Persistent storage using file handling
- Colorful, interactive console UI
- Multi-printer simulation

---

## Data Structures Used
- **Min Heap** – Priority queue for print jobs
- **Hash Map** – Job lookup by Job ID
- **Linked List** – User authentication
- **Arrays & Enums** – Job management

---

## Job Status Types
- Queued
- Delayed
- Completed
- Cancelled

---

## How to Run
1. Compile the program:
   ```bash
   g++ print_spooler.cpp -o spooler
