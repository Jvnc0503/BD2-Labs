select count(*) as count
from my_keyspace.temperature_day;

select *
from my_keyspace.temperature_day;

select *
from temperature_day
where sensor_id = 'c6bf2c02-2e0d-4d29-b946-18cf8c958179'
  and date = '2023-03-31';