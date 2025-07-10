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

select count(distinct date)
from temperature_measurements;

select count(*)
from temperature_measurements
where sensor_id = 'SENS003'
  and date = '2025-07-06';

select event_time, temperature, humidity
from temperature_measurements
where sensor_id = 'SENS003'
  and date = '2025-07-06'
order by event_time desc;

select sensor_id, avg(temperature) as avg_temp
from temperature_measurements
where date = '2025-07-06'
  and event_time >= '2025-07-06 00:00:00'
group by sensor_id;