if [ -f server/keys/server.key ] || [ -f server/keys/server.pem ];
then
    echo files exist
else
    echo generate...
    if [ ! -d server/keys/ ];
    then
        mkdir server/keys/
    fi
    openssl req  -nodes -new -x509  -keyout server/keys/server.key -out server/keys/server.pem
fi
