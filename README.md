Написал комплект из двух программ, одна из которых посылает данные
в разделяемую память, а вторая – читает эти данные. Поскольку механизм
разделяемой памяти не содержит средств синхронизации записи и чтения,
для синхронизации потребовалось применить механизм именованных семафоров.