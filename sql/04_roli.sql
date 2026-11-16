-- Роли базы данных: сотрудник, кадровик, админ.
-- Программа подключается одним пользователем Windows, поэтому вход в нее
-- проверяется по таблице Polzovateli (sql\07_polzovateli.sql).
-- Эти роли нужны, чтобы в SSMS было видно разграничение прав и их можно
-- было выдать отдельным учетным записям Windows.
USE OtpuskKompanii;
GO

IF DATABASE_PRINCIPAL_ID(N'rol_sotrudnik') IS NULL
    EXEC(N'CREATE ROLE rol_sotrudnik AUTHORIZATION dbo');
IF DATABASE_PRINCIPAL_ID(N'rol_kadrovik') IS NULL
    EXEC(N'CREATE ROLE rol_kadrovik AUTHORIZATION dbo');
IF DATABASE_PRINCIPAL_ID(N'rol_admin') IS NULL
    EXEC(N'CREATE ROLE rol_admin AUTHORIZATION dbo');
GO

-- Сотрудник только смотрит.
GRANT SELECT ON dbo.Otdely TO rol_sotrudnik;
GRANT SELECT ON dbo.Sotrudniki TO rol_sotrudnik;
GRANT SELECT ON dbo.Otpuska TO rol_sotrudnik;
GO

-- Кадровик ведет отделы, сотрудников и отпуска.
GRANT SELECT, INSERT, UPDATE, DELETE ON dbo.Otdely TO rol_kadrovik;
GRANT SELECT, INSERT, UPDATE, DELETE ON dbo.Sotrudniki TO rol_kadrovik;
GRANT SELECT, INSERT, UPDATE, DELETE ON dbo.Otpuska TO rol_kadrovik;
GO

-- Админ умеет то же самое; прошедшие отпуска разрешает программа.
GRANT SELECT, INSERT, UPDATE, DELETE ON dbo.Otdely TO rol_admin;
GRANT SELECT, INSERT, UPDATE, DELETE ON dbo.Sotrudniki TO rol_admin;
GRANT SELECT, INSERT, UPDATE, DELETE ON dbo.Otpuska TO rol_admin;
GO
