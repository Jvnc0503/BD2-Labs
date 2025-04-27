CREATE EXTENSION postgis;

DROP TABLE IF EXISTS cities;

CREATE TABLE cities
(
    id           int PRIMARY KEY,
    name         varchar(100),
    state_id     INT,
    state_code   varchar(10),
    state_name   varchar(100),
    country_id   INT,
    country_code varchar(10),
    country_name varchar(100),
    latitude     DOUBLE PRECISION,
    longitude    DOUBLE PRECISION,
    wikiDataId   varchar(15)
);

SELECT *
FROM cities;

ALTER TABLE cities
    ADD COLUMN ubicacion GEOGRAPHY(Point, 4326);

UPDATE cities
SET ubicacion = ST_SetSRID(ST_MakePoint(longitude, latitude), 4326);

DROP INDEX IF EXISTS idx_cities_geom_gist;
CREATE INDEX idx_cities_geom_gist ON cities USING GIST (ubicacion);

SELECT name, country_name
FROM cities
WHERE ST_DWithin(
              ubicacion::geography,
              ST_MakePoint(-78.91667, -8.08333)::geography,
              10000 -- metros
      );

SELECT c.name,
       c.country_name,
       ST_Distance(c.ubicacion::geography, qp.ubicacion::geography) AS distance
FROM cities c,
     (SELECT ST_SetSRID(ST_MakePoint(-78.91667, -8.08333), 4326) AS ubicacion) qp
ORDER BY c.ubicacion <-> qp.ubicacion
LIMIT 5;

ALTER TABLE cities
    ADD COLUMN ubicacion2 GEOGRAPHY(Point, 4326);

UPDATE cities
SET ubicacion2 = ubicacion;

EXPLAIN ANALYZE
SELECT name
FROM cities
WHERE ST_DWithin(
              ubicacion::geography,
              ST_MakePoint(-78.91667, -8.08333)::geography,
              100000
      );

EXPLAIN ANALYZE
SELECT name
FROM cities
WHERE ST_DWithin(
              ubicacion2::geography,
              ST_MakePoint(-78.91667, -8.08333)::geography,
              100000
      );

SELECT cities.name
FROM cities
WHERE ST_DWithin(ubicacion, ST_MakeEnvelope(
        -79.4742, -8.6159, -- Esquina inferior izquierda
        -78.4742, -7.6159, -- Esquina superior derecha
        4326), 10000);

SELECT c.name,
       c.country_name,
       ST_Distance(c.ubicacion::geography, qp.ubicacion::geography) AS distance
FROM cities c,
     (SELECT ST_SetSRID(ST_MakePoint(-12.04318, -77.02824), 4326) AS ubicacion) qp
ORDER BY c.ubicacion <-> qp.ubicacion
LIMIT 10;

CREATE TABLE cities20k
(
    id           int PRIMARY KEY,
    name         varchar(100),
    state_id     INT,
    state_code   varchar(10),
    state_name   varchar(100),
    country_id   INT,
    country_code varchar(10),
    country_name varchar(100),
    latitude     DOUBLE PRECISION,
    longitude    DOUBLE PRECISION,
    wikiDataId   varchar(15),
    ubicacion    GEOGRAPHY(Point, 4326),
    ubicacion2   GEOGRAPHY(Point, 4326)
);

CREATE TABLE cities40k
(
    id           int PRIMARY KEY,
    name         varchar(100),
    state_id     INT,
    state_code   varchar(10),
    state_name   varchar(100),
    country_id   INT,
    country_code varchar(10),
    country_name varchar(100),
    latitude     DOUBLE PRECISION,
    longitude    DOUBLE PRECISION,
    wikiDataId   varchar(15),
    ubicacion    GEOGRAPHY(Point, 4326),
    ubicacion2   GEOGRAPHY(Point, 4326)
);

CREATE TABLE cities60k
(
    id           int PRIMARY KEY,
    name         varchar(100),
    state_id     INT,
    state_code   varchar(10),
    state_name   varchar(100),
    country_id   INT,
    country_code varchar(10),
    country_name varchar(100),
    latitude     DOUBLE PRECISION,
    longitude    DOUBLE PRECISION,
    wikiDataId   varchar(15),
    ubicacion    GEOGRAPHY(Point, 4326),
    ubicacion2   GEOGRAPHY(Point, 4326)
);

CREATE TABLE cities80k
(
    id           int PRIMARY KEY,
    name         varchar(100),
    state_id     INT,
    state_code   varchar(10),
    state_name   varchar(100),
    country_id   INT,
    country_code varchar(10),
    country_name varchar(100),
    latitude     DOUBLE PRECISION,
    longitude    DOUBLE PRECISION,
    wikiDataId   varchar(15),
    ubicacion    GEOGRAPHY(Point, 4326),
    ubicacion2   GEOGRAPHY(Point, 4326)
);

INSERT INTO cities20k
SELECT *
FROM cities
LIMIT 20000;

INSERT INTO cities40k
SELECT *
FROM cities
LIMIT 40000;

INSERT INTO cities60k
SELECT *
FROM cities
LIMIT 60000;

INSERT INTO cities80k
SELECT *
FROM cities
LIMIT 80000;

DROP INDEX IF EXISTS idx_cities20k_geom_gist;
DROP INDEX IF EXISTS idx_cities40k_geom_gist;
DROP INDEX IF EXISTS idx_cities60k_geom_gist;
DROP INDEX IF EXISTS idx_cities80k_geom_gist;

ANALYSE cities20k;
ANALYSE cities40k;
ANALYSE cities60k;
ANALYSE cities80k;

CREATE INDEX idx_cities20k_geom_gist ON cities20k USING GIST (ubicacion);
CREATE INDEX idx_cities40k_geom_gist ON cities40k USING GIST (ubicacion);
CREATE INDEX idx_cities60k_geom_gist ON cities60k USING GIST (ubicacion);
CREATE INDEX idx_cities80k_geom_gist ON cities80k USING GIST (ubicacion);

VACUUM;
VACUUM FULL;

EXPLAIN ANALYZE
SELECT name, ubicacion <-> ST_MakePoint(-78.91667, -8.08333)::geography AS distance
FROM cities20k
WHERE ST_DWithin(
              ubicacion::geography,
              ST_MakePoint(-78.91667, -8.08333)::geography,
              1000000
      )
ORDER BY distance
LIMIT 10;

EXPLAIN ANALYZE
SELECT name, ubicacion2 <-> ST_MakePoint(-78.91667, -8.08333)::geography AS distance
FROM cities20k
WHERE ST_DWithin(
              ubicacion2::geography,
              ST_MakePoint(-78.91667, -8.08333)::geography,
              1000000
      )
ORDER BY distance
LIMIT 10;