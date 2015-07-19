SSL-Key is generated with:

openssl req -x509 -newkey rsa:2048 -keyout testdata/ssl/ppl6.key -out testdata/ssl/ppl6.cert -days 10000

PEM-Passphrase: bGl7R_3JkaRT=1+A


DH-Keys are generated with:
openssl dhparam -out testdata/ssl/dh_param_512.pem -2 512
openssl dhparam -out testdata/ssl/dh_param_1024.pem -2 1024



