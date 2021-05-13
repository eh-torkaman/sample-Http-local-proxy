
FROM ubuntu:18.04

ADD PROXY ./PROXY
WORKDIR ./PROXY

# tell the port number the container should expose
EXPOSE 5555

RUN ls
# run the command
CMD   ./proxy 5555 1 10
