FROM debian

RUN DEBIAN_FRONTEND=noninteractive apt update -y && apt install -y libasound2 libfreetype6 libcurl3-gnutls libgl1 libbluetooth3 libavahi-common3 libavahi-client3 libbluetooth-dev libx11-dev xvfb x11vnc fluxbox libatomic1

ARG USER_ID=1000
ARG GROUP_ID=1000
ARG REPOSITORY_OWNER
ARG IMAGE_NAME

LABEL org.opencontainers.image.source https://github.com/$REPOSITORY_OWNER/$IMAGE_NAME

RUN addgroup --gid $GROUP_ID user
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user

COPY ./Chataigne.AppDir/usr/lib/* /usr/lib/
COPY ./Chataigne.AppDir/usr/bin/server.key /usr/bin/
COPY ./Chataigne.AppDir/usr/bin/server.crt /usr/bin/
COPY ./build/Chataigne /home/user/

RUN chown -R user:user /home/user
USER user
WORKDIR /home/user

ENV DISPLAY :1
CMD rm -f /tmp/.X*-lock && Xvfb :1 -screen 0 1280x1024x16 & fluxbox & x11vnc -forever & ./Chataigne
