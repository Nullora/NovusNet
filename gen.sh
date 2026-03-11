echo "Generating NovusNet TLS certificates..."
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/CN=localhost"
echo "Done. cert.pem and key.pem created."
echo "Place them in the same directory as your server binary."