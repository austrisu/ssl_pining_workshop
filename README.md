# SSL Pinning & Bypass – Mini Workshop

Small lab to practice SSL pinning and different bypass methods.

## 1. Overview

- `server.py` – simple HTTPS server exposing `/secret`
- `secret.cpp` – C++ client using libcurl + **public key pinning**
- Pin type: `CURLOPT_PINNEDPUBLICKEY` with SHA256 of server’s public key

## 2. Requirements

```sh
sudo apt-get install python3 openssl g++ libcurl4-openssl-dev libssl-dev
```

## 3. Generate certificate & key

Self-signed cert for localhost
```sh
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/CN=localhost"
```

Get SHA-256 of the public key (for pinning):
```sh
openssl x509 -in cert.pem -pubkey -noout > pubkey.pem

#SHA-256 hash of DER-encoded public key
openssl pkey -pubin -in pubkey.pem -outform DER   | openssl dgst -sha256
```

This hash (base64-encoded) is what ends up in:

```cpp
curl_easy_setopt(curl,
  CURLOPT_PINNEDPUBLICKEY,
  "sha256//SNfBiT+ZqY25IdLbYlSfzNZUe/mxOxgnGti1SBmW5yY="
);
```

## 4. Extracting a pin from *any* HTTPS server

Example (also works for this lab server):

```sh
# Grab server certificate
openssl s_client -servername localhost -connect localhost:1337   < /dev/null | sed -n "/-----BEGIN/,/-----END/p" > local_server.pem

# Extract public key
openssl x509 -in local_server.pem -pubkey -noout > local_server.pubkey.pem

# Convert to DER
openssl asn1parse -noout -inform pem   -in local_server.pubkey.pem   -out local_server.pubkey.der

# SHA-256 and base64 for CURLOPT_PINNEDPUBLICKEY
openssl dgst -sha256 -binary local_server.pubkey.der | openssl base64
```

## 5. Run the HTTPS server

```sh
python3 server.py
```

Server listens on:

- `https://127.0.0.1:1337/secret`
- Response body: `Here_you_go,_the_big_secret`

## 6. Build the C++ client

```sh
g++ secret.cpp -o secret -lcurl -lcrypto
```

Run:

```sh
./secret
```

If pinning fails (wrong cert/public key), libcurl returns an error and the connection is rejected. Check curl error codes here: https://curl.se/libcurl/c/libcurl-errors.html


## 8. References

- curl TLS pinning guide:  
  https://everything.curl.dev/usingcurl/tls/pinning.html
- libcurl `CURLOPT_PINNEDPUBLICKEY`:  
  https://curl.se/libcurl/c/CURLOPT_PINNEDPUBLICKEY.html