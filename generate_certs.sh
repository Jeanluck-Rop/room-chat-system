#!/bin/bash
CERT_FILE="src/server/certi.pem"
KEY_FILE="src/server/key.pem"

if [ ! -f "$CERT_FILE" ] || [ ! -f "$KEY_FILE" ]; then
  echo "[INFO] Generating self-signed certificate..."
  mkdir -p src/server
  openssl req -x509 -newkey rsa:2048 -nodes \
    -keyout "$KEY_FILE" \
    -out "$CERT_FILE" \
    -days 365 \
    -subj "/C=MX/ST=CDMX/L=CDMX/O=RoomChat/CN=localhost"
else
  echo "[INFO] Certificates already exist."
fi
