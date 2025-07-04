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