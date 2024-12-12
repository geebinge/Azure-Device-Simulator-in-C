FROM library/ubuntu:20.04 AS builder

RUN set -ex \
    && apt-get update \
    && apt-get install -y \
        curl \
	&& rm -rf /var/lib/apt/lists/*
	
WORKDIR /workdir 

RUN set -ex \
    && curl -L -o azcopy.tar.gz \
    https://aka.ms/downloadazcopy-v10-linux \
    && tar -xf azcopy.tar.gz --strip-components=1 \
    && rm -f azcopy.tar.gz

FROM ubuntu

RUN set -ex \
    && apt-get update \
    && apt-get install -y \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /workdir/azcopy /usr/local/bin

RUN apt-get update
RUN apt-get upgrade -y 
RUN apt-get install libcurl4-openssl-dev libssl-dev uuid-dev -y 
RUN apt-get install openssl -y 
RUN apt-get install wget -y 

RUN wget http://archive.ubuntu.com/ubuntu/pool/main/o/openssl/libssl1.1_1.1.1f-1ubuntu2_amd64.deb
RUN dpkg -i libssl1.1_1.1.1f-1ubuntu2_amd64.deb

 
COPY ./dev_simulation /root/dev_simulation
RUN chmod -R 755 /root/dev_simulation/*

CMD [ "/root/dev_simulation/sml_device.sh" ]








 
 
 
  

