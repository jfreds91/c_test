# Specify x86-compatible base image
FROM --platform=linux/amd64 ubuntu:20.04

# Install essential tools
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    gdb \
    curl \
    git \
    && apt-get clean
