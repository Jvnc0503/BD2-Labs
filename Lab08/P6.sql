-- P6:
create database lab8;

set search_path to lab8;

create extension if not exists vector;

drop table if exists embeddings_with_vector;

create table if not exists embeddings_with_vector
(
    id          serial primary key,
    person_name text,
    embedding   VECTOR(128)
);

--Insertar datos desde el notebook

create index if not exists idx_vector_ivfflat
    on embeddings_with_vector
        using ivfflat (embedding vector_cosine_ops)
    with (lists = 100);

set ivfflat.probes = 10;

-- hacer la query desde python