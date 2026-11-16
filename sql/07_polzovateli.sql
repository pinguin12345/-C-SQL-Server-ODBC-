-- Пользователи программы: логин, хеш пароля SHA-256, роль.
-- Пароли не хранятся открытым текстом.
-- Демонстрационные пароли набраны латиницей, чтобы UTF-8 в программе
-- и VARCHAR в SQL давали одинаковые байты.
USE OtpuskKompanii;
GO

IF OBJECT_ID(N'dbo.Polzovateli', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.Polzovateli (
        Id INT NOT NULL PRIMARY KEY,
        Login NVARCHAR(30) NOT NULL UNIQUE,
        ParolHash NVARCHAR(64) NOT NULL,
        Rol NVARCHAR(20) NOT NULL,
        CONSTRAINT CK_Polzovateli_Rol CHECK (Rol IN (N'сотрудник', N'кадровик', N'админ'))
    );
    PRINT N'Таблица Polzovateli создана.';
END
GO

IF NOT EXISTS (SELECT 1 FROM dbo.Polzovateli)
BEGIN
    INSERT INTO dbo.Polzovateli (Id, Login, ParolHash, Rol) VALUES
        (1, N'sotrudnik',
         CONVERT(NVARCHAR(64), HASHBYTES('SHA2_256', CAST(N'sotrudnik123' AS VARCHAR(50))), 2),
         N'сотрудник'),
        (2, N'kadrovik',
         CONVERT(NVARCHAR(64), HASHBYTES('SHA2_256', CAST(N'kadrovik123' AS VARCHAR(50))), 2),
         N'кадровик'),
        (3, N'admin',
         CONVERT(NVARCHAR(64), HASHBYTES('SHA2_256', CAST(N'admin123' AS VARCHAR(50))), 2),
         N'админ');
    PRINT N'Три пользователя добавлены.';
END
GO

SELECT Login, Rol FROM dbo.Polzovateli ORDER BY Id;
GO

-- Программа читает эту таблицу при входе, поэтому читать ее должны все роли.
IF DATABASE_PRINCIPAL_ID(N'rol_sotrudnik') IS NOT NULL
BEGIN
    GRANT SELECT ON dbo.Polzovateli TO rol_sotrudnik;
    GRANT SELECT ON dbo.Polzovateli TO rol_kadrovik;
    GRANT SELECT ON dbo.Polzovateli TO rol_admin;
END
GO
