FROM node:18-slim
WORKDIR /app
COPY package*.json ./
ADD api ./
RUN npm install --production
CMD node app.js