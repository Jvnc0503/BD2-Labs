drop table if exists temperature_measurements;

create table if not exists temperature_measurements
(
    sensor_id   varchar(20),
    date        varchar(10),
    event_time  timestamp,
    temperature double precision,
    humidity    double precision,
    primary key (sensor_id, date, event_time)
);