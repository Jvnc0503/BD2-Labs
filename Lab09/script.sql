create schema if not exists lab9;

set search_path to lab9;

create table if not exists employees
(
    emp_no     int,
    birth_date date,
    first_name varchar(14),
    last_name  varchar(16),
    gender     character(1),
    hire_date  date,
    dept_no    varchar(5),
    from_date  date
);

drop table if exists employees;

create table if not exists salaries
(
    emp_no    int,
    salary    int,
    from_date date,
    to_date   date
);

drop table if exists employees;

set enable_partition_pruning = on;

--P1
select count(*)
from employees;

select dept_no, count(*) as count
from employees
group by dept_no
order by dept_no;

drop table if exists employees_1;

create table if not exists employees_1
(
    emp_no     int,
    birth_date date,
    first_name varchar(14),
    last_name  varchar(16),
    gender     character(1),
    hire_date  date,
    dept_no    varchar(5),
    from_date  date
) partition by list (dept_no);

create table employees_p1 partition of employees_1
    for values in ('d005', 'd006');

create table employees_p2 partition of employees_1
    for values in ('d004', 'd008', 'd003');

create table employees_p3 partition of employees_1
    for values in ('d007', 'd009', 'd001', 'd002');

select count(*)
from employees_p1
union
select count(*)
from employees_p2
union
select count(*)
from employees_p3;

vacuum full employees;
vacuum full employees_1;

explain analyse
select *
from employees
where dept_no = 'd005';

explain analyse
select *
from employees_p1
where dept_no = 'd005';

--P2

drop table if exists employees_2;

create table if not exists employees_2
(
    emp_no     int,
    birth_date date,
    first_name varchar(14),
    last_name  varchar(16),
    gender     character(1),
    hire_date  date,
    dept_no    varchar(5),
    from_date  date
) partition by range (date_part('year', hire_date));

select date_part('year', hire_date) as year, count(*) as count
from employees
group by year
order by year;

select '1987 minor equal' as range, count(*) as count
from employees
where date_part('year', hire_date) <= 1987
union all
select '1988-1991' as range, count(*) as count
from employees
where date_part('year', hire_date) between 1988 and 1991
union all
select '1992 greater equal' as range, count(*) as count
from employees
where date_part('year', hire_date) >= 1992
order by range;

create table employees2_p1 partition of employees_2
    for values from (minvalue) to (1988);

create table employees2_p2 partition of employees_2
    for values from (1988) to (1992);

create table employees2_p3 partition of employees_2
    for values from (1992) to (maxvalue);

select 'p1' as partition, count(*) as count
from employees2_p1
union all
select 'p2' as partition, count(*) as count
from employees2_p2
union all
select 'p3' as partition, count(*) as count
from employees2_p3
union all
select 'union' as partition, sum(total.count)
from (select count(*) as count
      from employees2_p1
      union all
      select count(*) as count
      from employees2_p2
      union all
      select count(*) as count
      from employees2_p3) as total
union all
select 'original' as partition, count(*) as count
from employees;
