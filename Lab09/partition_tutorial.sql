create schema if not exists partition_tutorial;

set search_path to partition_tutorial;

CREATE TABLE persona
(
    id        int          not null,
    fecha_nac date         not null,
    cod_pais  character(2) not null,
    nombre    varchar(30)
) PARTITION BY LIST (cod_pais);

-- Partición para personas viviendo en Perú
CREATE TABLE persona_pe PARTITION OF persona FOR VALUES IN ('PE');

-- Partición para personas viviendo en Latinoamérica
CREATE TABLE persona_lat PARTITION OF persona FOR VALUES IN ('EC', 'BO', 'CH', 'AR',
    'CO', 'BR', 'VE');

-- Partición para el resto de personas del mundo
CREATE TABLE persona_resto PARTITION OF persona DEFAULT;

INSERT INTO persona (id, fecha_nac, cod_pais, nombre)
VALUES (1, '2000-01-01', 'PE', 'Heider'),
       (2, '2000-02-02', 'AR', 'Maria'),
       (3, '2001-03-03', 'EC', 'Roberto');

SELECT *
FROM persona;

SELECT *
FROM persona
WHERE cod_pais = 'EC'
ORDER BY nombre;

SELECt *
FROM persona_lat;

CREATE INDEX global_persona_fecha_idx ON persona (fecha_nac);

EXPLAIN ANALYSE
SELECT *
FROM persona
ORDER BY fecha_nac;

-- Eliminar particion
ALTER TABLE persona
    DETACH PARTITION persona_pe;
DROP TABLE persona_pe;

-- Unir otra particion
CREATE TABLE persona_pe
(
    id        int          not null,
    fecha_nac date         not null,
    cod_pais  character(2) not null,
    nombre    varchar(30)
);

ALTER TABLE persona
    ATTACH PARTITION persona_pe FOR VALUES IN ('PE');

drop schema if exists partition_tutorial cascade;

create schema if not exists partition_tutorial;

set search_path to partition_tutorial;