CREATE TABLE noticias
(
    id           SERIAL PRIMARY KEY,
    url          TEXT,
    contenido    TEXT,
    categoria    VARCHAR(50),
    bag_of_words JSONB
);

CREATE TABLE stopwords
(
    id   SERIAL PRIMARY KEY,
    word TEXT UNIQUE NOT NULL
);

SELECT *
FROM noticias;

SELECT *
FROM stopwords;

SELECT COUNT(*)
FROM noticias;

CREATE TABLE noticias600
(
    id           SERIAL PRIMARY KEY,
    url          TEXT,
    contenido    TEXT,
    categoria    VARCHAR(50),
    bag_of_words JSONB
);

CREATE TABLE noticias300
(
    id           SERIAL PRIMARY KEY,
    url          TEXT,
    contenido    TEXT,
    categoria    VARCHAR(50),
    bag_of_words JSONB
);

CREATE TABLE noticias150
(
    id           SERIAL PRIMARY KEY,
    url          TEXT,
    contenido    TEXT,
    categoria    VARCHAR(50),
    bag_of_words JSONB
);

INSERT INTO noticias600 (url, contenido, categoria, bag_of_words)
SELECT url, contenido, categoria, bag_of_words
FROM noticias
ORDER BY id
LIMIT 600;

INSERT INTO noticias300 (url, contenido, categoria, bag_of_words)
SELECT url, contenido, categoria, bag_of_words
FROM noticias
ORDER BY id
LIMIT 300;

INSERT INTO noticias150 (url, contenido, categoria, bag_of_words)
SELECT url, contenido, categoria, bag_of_words
FROM noticias
ORDER BY id
LIMIT 150;