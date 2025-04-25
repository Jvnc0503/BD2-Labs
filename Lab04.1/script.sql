-- Tabla con índice B+ Tree
CREATE TABLE temperatura_btree_desordenada
(
    id          SERIAL PRIMARY KEY,
    sensor_id   INT,
    fecha       TIMESTAMP,
    temperatura DECIMAL(5, 2)
);

-- Tabla con índice BRIN
CREATE TABLE temperatura_brin_desordenada
(
    id          SERIAL PRIMARY KEY,
    sensor_id   INT,
    fecha       TIMESTAMP,
    temperatura DECIMAL(5, 2)
);

INSERT INTO temperatura_btree_desordenada (sensor_id, fecha, temperatura)
SELECT (RANDOM() * 9 + 1)::INT,
       TIMESTAMP '2024-01-01' + RANDOM() * (TIMESTAMP '2024-12-31' - TIMESTAMP '2024-01-01'),
       ROUND(CAST(20 + RANDOM() * 10 AS NUMERIC), 2)
FROM generate_series(1, 525600) AS s(i);

INSERT INTO temperatura_brin_desordenada
SELECT *
FROM temperatura_btree_desordenada;

-- Índice B+ Tree
CREATE INDEX idx_fecha_btree ON temperatura_btree_desordenada (fecha);
-- Índice BRIN
CREATE INDEX idx_fecha_brin ON temperatura_brin_desordenada USING BRIN (fecha);

-- B+ Tree
EXPLAIN ANALYZE
SELECT *
FROM temperatura_btree_desordenada
WHERE fecha BETWEEN '2024-06-01 00:00:00' AND '2024-06-01 01:00:00';
-- BRIN
EXPLAIN ANALYZE
SELECT *
FROM temperatura_brin_desordenada
WHERE fecha BETWEEN '2024-06-01 00:00:00' AND '2024-06-01 01:00:00';

DROP INDEX idx_fecha_brin;
CREATE INDEX idx_fecha_brin ON temperatura_brin_desordenada
    USING BRIN (fecha) WITH (pages_per_range = 4);
SET ENABLE_SEQSCAN = OFF;
SET ENABLE_BITMAPSCAN = OFF;
SET ENABLE_INDEXSCAN = OFF;
VACUUM;
VACUUM temperatura_btree_desordenada;
VACUUM temperatura_brin_desordenada;
VACUUM FULL;
-- Test case rango 1 minuto
-- B+ Tree
EXPLAIN ANALYSE
SELECT *
FROM temperatura_btree_desordenada
WHERE fecha BETWEEN '2024-03-29 00:00:00' AND '2024-05-29 00:00:00';
-- BRIN
EXPLAIN ANALYZE
SELECT *
FROM temperatura_brin_desordenada
WHERE fecha BETWEEN '2024-03-29 00:00:00' AND '2024-05-29 00:00:00';