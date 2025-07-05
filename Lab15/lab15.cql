drop table if exists temperature;

create table if not exists temperature
(
    sensor_id   text,
    event_time  timestamp,
    temperature double,
    primary key (sensor_id, event_time)
);

insert into temperature(sensor_id, event_time, temperature)
values ('1234ABCD', '2018-04-03 07:01:00', 25);
insert into temperature(sensor_id, event_time, temperature)
values ('1234ABCD', '2018-04-03 07:02:00', 24);
insert into temperature(sensor_id, event_time, temperature)
values ('1234ABCD', '2018-04-03 07:03:00', 24);
insert into temperature(sensor_id, event_time, temperature)
values ('1234ABCD', '2018-04-03 07:04:00', 25);

select temperature
from temperature
where sensor_id = '1234ABCD'
  and event_time > '2018-04-03 07:01:00'
  and event_time < '2018-04-03 07:04:00';

drop table if exists temperature_day;

create table if not exists temperature_day
(
    sensor_id   text,
    date        text,
    event_time  timestamp,
    temperature double,
    primary key (sensor_id, event_time)
);

insert into temperature_day(sensor_id, date, event_time, temperature)
values ('1234ABCD', '2018-04-03', '2018-04-03 07:01:00', 25);
insert into temperature_day(sensor_id, date, event_time, temperature)
values ('1234ABCD', '2018-04-03', '2018-04-03 07:02:00', 24);
insert into temperature_day(sensor_id, date, event_time, temperature)
values ('1234ABCD', '2018-04-03', '2018-04-03 07:03:00', 24);
insert into temperature_day(sensor_id, date, event_time, temperature)
values ('1234ABCD', '2018-04-03', '2018-04-03 07:04:00', 25);

select *
from temperature_day
where sensor_id = '1234ABCD'
  and date = '2018-04-03'
    allow filtering;

drop table if exists latest_temperatures;

create table if not exists latest_temperatures
(
    sensor_id   text,
    event_time  timestamp,
    temperature double,
    primary key (sensor_id, event_time)
) with default_time_to_live = 259200;

INSERT INTO latest_temperatures(sensor_id, event_time, temperature)
VALUES ('1234ABCD', '2018-04-03 07:03:00', 22);
INSERT INTO latest_temperatures(sensor_id, event_time, temperature)
VALUES ('1234ABCD', '2018-04-03 07:02:00', 23);
INSERT INTO latest_temperatures(sensor_id, event_time, temperature)
VALUES ('1234ABCD', '2018-04-03 07:01:00', 24) USING TTL 86400;
INSERT INTO latest_temperatures(sensor_id, event_time, temperature)
VALUES ('1234ABCD', '2018-04-03 07:04:00', 24) USING TTL 86400;

select sensor_id, event_time, temperature, ttl(temperature)
from latest_temperatures;

create table if not exists sensor_readings
(
    measurement_type text,
    sensor_id        text,
    date             text,
    event_time       timestamp,
    measurement      double,
    primary key ( (measurement_type, sensor_id, date), event_time )
) with clustering order by (event_time desc) and
        default_time_to_live = 604800;