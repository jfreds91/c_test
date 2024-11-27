# Specify x86-compatible base image
FROM --platform=linux/amd64 ubuntu:20.04

# Set the timezone to prevent tzdata from prompting
ENV DEBIAN_FRONTEND=noninteractive
RUN ln -fs /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    apt-get update && apt-get install -y \
    build-essential \
    gcc \
    gdb \
    tzdata \
    curl \
    git \
    && dpkg-reconfigure --frontend noninteractive tzdata \
    && apt-get clean
