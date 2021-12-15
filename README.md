# Security Proxy Server
## Type of project

Project based on Unix Sockets (TCP and UDP)

## ISABELA App

IoT Student Advisor and Lifestyle Analyzer

Problem: Collection of private data about students life (example: sleep patterns, social activities, location, sound, etc)

Security already implemented: Data is stored anonimously stored in a database

## Challange proposed

2 Applications
- 1 Privacy Proxy Server
- 1 Client Application

Client <---TCP/IP---> Privacy Server <---HTTP---> Isabela

Privacy Server only sends clients own data. Provides privacy and security.

Possible Solution: 
- Encryption Algorithms (Only the user that owns the data can decrypt it)
- Assymetric Encryption?

## Deadlines

November, 19 - Client and Server exchanging plane text messages
December, 16 - Final delivery of code and report

## Solution Implemented

Types of Encryption and Security Measures used:
-> Password Hashing with Salt using Bcrypt Algorithm
-> Asymmetric Encryption to exchange Symmetric Encryption Key and IV (Generated two Public/Private Key Pairs for each execution)
-> Symmetric Encryption of Messages between the Server and the Client
-> Cryptographically-Secure Pseudo-Random Number Generator to create new Key/IV Pair for Symmetric Encryption in each execution
