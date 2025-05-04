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