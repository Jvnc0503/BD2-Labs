create extension cube;

drop table if exists vectors;

create table if not exists vectors
(
    id            serial,
    vector_lineal cube,
    vector_gist   cube
);

--Populate the table with random data
insert into vectors(id, vector_lineal)
select id,
       cube(array [
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000),
           round(random() * 1000)
           ])
from generate_series(1, 100000) as id;

update vectors
set vector_gist = vector_lineal;

--Create rtree index
create index idx_vector on vectors using gist (vector_gist);

VACUUM FULL vectors;
VACUUM vectors;

--KNN without index
explain analyze
select id, vector_lineal, cube_distance(vector_lineal, '(638, 616, 153, 907)') as distance
from vectors
order by vector_lineal <-> '(636, 616, 153, 907)'
limit 5;

--KNN with index
explain analyze
select id, vector_gist, cube_distance(vector_gist, '(638, 616, 153, 907)') as distance
from vectors
order by vector_gist <-> '(636, 616, 153, 907)'
limit 5;