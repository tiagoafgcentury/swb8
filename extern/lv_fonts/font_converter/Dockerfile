FROM node:22

WORKDIR /app

RUN apt update; apt install -y git ; git clone https://github.com/lvgl/lv_font_conv.git

RUN npm i lv_font_conv -g
