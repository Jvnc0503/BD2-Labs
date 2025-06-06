CREATE EXTENSION IF NOT EXISTS cube;

SET enable_seqscan = ON;

create table if not exists embeddings
(
    id              serial primary key,
    person_name     text,
    cube_sequential cube,
    cube_gist       cube
);

-- (insertar usando el notebook)

CREATE INDEX IF NOT EXISTS idx_cube_gist ON embeddings USING gist (cube_gist);

select *
from embeddings
limit 10;