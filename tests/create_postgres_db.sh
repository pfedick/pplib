#!/bin/sh

DBHOST=${DBHOST:=localhost}
DBPORT=${DBPORT:=5432}
DBNAME=${DBNAME:=ppl7}
DBUSER=${DBUSER:=postgres}
PPL7USER=${PPL7USER:=ppl7}


# Vorhandene Datenbank droppen und neuanlegen

psql -h ${DBHOST} -p ${DBPORT} -U ${DBUSER} -c "SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE datname='${DBNAME}'"
if [ $? -ne 0 ] ; then
    echo "ERROR: psql-Fehler, Connections konnten nicht gedroppt werden"
    exit 1
fi


psql -h ${DBHOST} -p ${DBPORT} -U ${DBUSER} -c "drop database ${DBNAME}"
if [ $? -ne 0 ] ; then
    echo "INFO: psql-Fehler, Datenbank ${DBNAME} konnten nicht gedroppt werden"
fi

cat <<EOF | psql -h ${DBHOST} -p ${DBPORT} -U ${DBUSER}
CREATE DATABASE ${DBNAME}
  WITH OWNER      = ${PPL7USER}
       ENCODING   = 'UTF8'
       TABLESPACE = pg_default
       LC_COLLATE = 'de_DE.UTF-8'
       LC_CTYPE   = 'de_DE.UTF-8'
       TEMPLATE   = template0
       CONNECTION LIMIT = -1;

GRANT CONNECT, TEMPORARY ON DATABASE ${DBNAME} TO public;
GRANT ALL ON DATABASE ${DBNAME} TO ${PPL7USER};
EOF
if [ $? -ne 0 ] ; then
    echo "INFO: psql-Fehler, Datenbank ${DBNAME} konnten nicht gedroppt werden"
fi



cat testdata/database/postgresql.sql | psql -h ${DBHOST} -p ${DBPORT} -U ${PPL7USER} -d ${DBNAME}