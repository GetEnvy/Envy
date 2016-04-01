; *** Inno Setup version 5.5.0+ Ukrainian messages (Envy: none)
;
; Translation made by Oleh Malyy
; mals@dreamsoft.ssft.net
; http://www.dreamsoft-sg.com/
;
; Author: Dmitry Onischuk
; E-Mail: mr.lols@yandex.ua
; Web: http://counter-strike.com.ua/
;
; Version 2012.12.14
;
; Based on translation by Vitaliy Levchenko v.4.0.5

[LangOptions]
LanguageName=Ukrainian
LanguageID=$0422
LanguageCodePage=1251

[Messages]

; *** Application titles
SetupAppTitle=Встановлення
SetupWindowTitle=Встановлення — %1
UninstallAppTitle=Видалення
UninstallAppFullTitle=Видалення — %1

; *** Misc. common
InformationTitle=Інформація
ConfirmTitle=Підтвердження
ErrorTitle=Помилка

; *** SetupLdr messages
SetupLdrStartupMessage=Ця програма встановить %1 на ваш комп'ютер, бажаєте продовжити?
LdrCannotCreateTemp=Неможливо створити тимчасовий файл. Встановлення перервано
LdrCannotExecTemp=Неможливо виконати файл в тимчасовій папці. Встановлення перервано

; *** Startup error messages
LastErrorMessage=%1.%n%nПомилка %2: %3
SetupFileMissing=Файл %1 відсутній в папці встановлення. Будь ласка, виправте цю помилку або отримайте нову копію програми.
SetupFileCorrupt=Файли встановлення пошкоджені. Будь ласка, отримайте нову копію програми.
SetupFileCorruptOrWrongVer=Файли встановлення пошкоджені або несумісні з цією версією програми встановлення. Будь ласка, виправте цю помилку або отримайте нову копію програми.
InvalidParameter=Командний рядок містить недопустимий параметр:%n%n%1
SetupAlreadyRunning=Програма встановлення вже запущена.
WindowsVersionNotSupported=Ця програма не підтримує версію Windows, встановлену на цьому комп'ютері.
WindowsServicePackRequired=Ця програма вимагає %1 Service Pack %2 або більш пізню версію.
NotOnThisPlatform=Ця програма не буде працювати під %1.
OnlyOnThisPlatform=Ця програма повинна бути відкрита під %1.
OnlyOnTheseArchitectures=Ця програма може бути встановлена лише на комп'ютерах під управлінням Windows для наступних архітектур процесорів:%n%n%1
MissingWOW64APIs=В вашій версії Windows відсутні функції для виконання 64-бітного встановлення. Щоб усунути цю проблему встановіть пакет оновлень Service Pack %1.
WinVersionTooLowError=Ця програма вимагає %1 версії %2 або більш пізню версію.
WinVersionTooHighError=Ця програма не може бути встановлена на %1 версії %2 або більш пізню версію.
AdminPrivilegesRequired=Щоб встановити цю програму ви повинні увійти до системи як адміністратор.
PowerUserPrivilegesRequired=Щоб встановити цю програму ви повинні увійти до системи як адміністратор або як член групи «Досвідчені користувачі».
SetupAppRunningError=Виявлено, що %1 вже відкрита.%n%nБудь ласка, закрийте всі копії програми та натисніть «OK» для продовження, або «Скасувати» для виходу.
UninstallAppRunningError=Виявлено, що %1 вже відкрита.%n%nБудь ласка, закрийте всі копії програми та натисніть «OK» для продовження, або «Скасувати» для виходу.

; *** Misc. errors
ErrorCreatingDir=Програмі встановлення не вдалося створити папку "%1"
ErrorTooManyFilesInDir=Програмі встановлення не вдалося створити файл в папці "%1", тому що в ньому занадто багато файлів

; *** Setup common messages
ExitSetupTitle=Вихід з програми встановлення
ExitSetupMessage=Встановлення не закінчено. Якщо ви вийдете зараз, програму не буде встановлено.%n%nВи можете відкрити програму встановлення в інший час.%n%nВийти з програми встановлення?
AboutSetupMenuItem=&Про програму встановлення...
AboutSetupTitle=Про програму встановлення
AboutSetupMessage=%1 версія %2%n%3%n%n%1 домашня сторінка:%n%4
AboutSetupNote=
TranslatorNote=Ukrainian translation

; *** Buttons
ButtonBack=< &Назад
ButtonNext=&Далі >
ButtonInstall=&Встановити
ButtonOK=OK
ButtonCancel=Скасувати
ButtonYes=&Так
ButtonYesToAll=Так для &Всіх
ButtonNo=&Ні
ButtonNoToAll=Н&і для Всіх
ButtonFinish=&Готово
ButtonBrowse=&Огляд...
ButtonWizardBrowse=О&гляд...
ButtonNewFolder=&Створити папку

; *** "Select Language" dialog messages
SelectLanguageTitle=Виберіть мову встановлення
SelectLanguageLabel=Виберіть мову, яка буде використовуватися під час встановлення:

; *** Common wizard text
ClickNext=Натисніть «Далі», щоб продовжити, або «Скасувати» для виходу з програми встановлення.
BeveledLabel=
BrowseDialogTitle=Огляд папок
BrowseDialogLabel=Виберіть папку зі списку та натисніть «ОК».
NewFolderName=Нова папка

; *** "Welcome" wizard page
WelcomeLabel1=Ласкаво просимо до програми встановлення [name].
WelcomeLabel2=Ця програма встановить [name/ver] на ваш комп’ютер.%n%nРекомендується закрити всі інші програми перед продовженням.

; *** "Password" wizard page
WizardPassword=Пароль
PasswordLabel1=Ця програма встановлення захищена паролем.
PasswordLabel3=Будь ласка, введіть пароль та натисніть «Далі», щоб продовжити. Пароль чутливий до регістру.
PasswordEditLabel=&Пароль:
IncorrectPassword=Ви ввели неправильний пароль. Будь ласка, спробуйте ще раз.

; *** "License Agreement" wizard page
WizardLicense=Ліцензійна угода
LicenseLabel=Будь ласка, прочитайте ліцензійну угоду.
LicenseLabel3=Будь ласка, прочитайте ліцензійну угоду. Ви повинні прийняти умови цієї угоди, перш ніж продовжити встановлення.
LicenseAccepted=Я &приймаю умови угоди
LicenseNotAccepted=Я &не приймаю умови угоди

; *** "Information" wizard pages
WizardInfoBefore=Інформація
InfoBeforeLabel=Будь ласка, прочитайте наступну важливу інформацію, перш ніж продовжити.
InfoBeforeClickLabel=Якщо ви готові продовжити встановлення, натисніть «Далі».
WizardInfoAfter=Інформація
InfoAfterLabel=Будь ласка, прочитайте наступну важливу інформацію, перш ніж продовжити.
InfoAfterClickLabel=Якщо ви готові продовжити встановлення, натисніть «Далі».

; *** "User Information" wizard page
WizardUserInfo=Інформація про користувача
UserInfoDesc=Будь ласка, введіть дані про себе.
UserInfoName=&Ім’я користувача:
UserInfoOrg=&Організація:
UserInfoSerial=&Серійний номер:
UserInfoNameRequired=Ви повинні ввести ім'я.

; *** "Select Destination Location" wizard page
WizardSelectDir=Вибір шляху встановлення
SelectDirDesc=Куди ви бажаєте встановити [name]?
SelectDirLabel3=Програма встановить [name] у наступну папку.
SelectDirBrowseLabel=Натисніть «Далі», щоб продовжити. Якщо ви бажаєте вибрати іншу папку, натисніть «Огляд».
DiskSpaceMBLabel=Необхідно як мінімум [mb] Mб вільного дискового простору.
CannotInstallToNetworkDrive=Встановлення не може проводитися на мережевий диск.
CannotInstallToUNCPath=Встановлення не може проводитися по мережевому шляху.
InvalidPath=Ви повинні вказати повний шлях з буквою диску, наприклад:%n%nC:\APP%n%nабо в форматі UNC:%n%n\\сервер\ресурс
InvalidDrive=Обраний Вами диск чи мережевий шлях не існує, або не доступний. Будь ласка, виберіть інший.
DiskSpaceWarningTitle=Недостатньо дискового простору
DiskSpaceWarning=Для встановлення необхідно як мінімум %1 Кб вільного простору, а на вибраному диску доступно лише %2 Кб.%n%nВи все одно бажаєте продовжити?
DirNameTooLong=Ім'я папки або шлях до неї перевищують допустиму довжину.
InvalidDirName=Вказане ім’я папки недопустиме.
BadDirName32=Ім'я папки не може включати наступні символи:%n%n%1
DirExistsTitle=Папка існує
DirExists=Папка:%n%n%1%n%nвже існує. Ви все одно бажаєте встановити в цю папку?
DirDoesntExistTitle=Папка не існує
DirDoesntExist=Папка:%n%n%1%n%nне існує. Ви бажаєте створити її?

; *** "Select Components" wizard page
WizardSelectComponents=Вибір компонентів
SelectComponentsDesc=Які компоненти ви бажаєте встановити?
SelectComponentsLabel2=Виберіть компоненти які ви бажаєте встановити; зніміть відмітку з компонентів які ви не бажаєте встановлювати. Натисніть «Далі», щоб продовжити.
FullInstallation=Повне встановлення
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Компактне встановлення
CustomInstallation=Вибіркове встановлення
NoUninstallWarningTitle=Компоненти існують
NoUninstallWarning=Виявлено, що наступні компоненти вже встановленні на вашому комп’ютері:%n%n%1%n%nВідміна вибору цих компонентів не видалить їх.%n%nВи бажаєте продовжити?
ComponentSize1=%1 Kб
ComponentSize2=%1 Mб
ComponentsDiskSpaceMBLabel=Даний вибір вимагає як мінімум [mb] Mб дискового простору.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Вибір додаткових завдань
SelectTasksDesc=Які додаткові завдання ви бажаєте виконати?
SelectTasksLabel2=Виберіть додаткові завдання які програма встановлення [name] повинна виконати, потім натисніть «Далі».

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Вибір папки в меню «Пуск»
SelectStartMenuFolderDesc=Де ви бажаєте створити ярлики?
SelectStartMenuFolderLabel3=Програма встановлення створить ярлики у наступній папці меню «Пуск».
SelectStartMenuFolderBrowseLabel=Натисніть «Далі», щоб продовжити. Якщо ви бажаєте вибрати іншу папку, натисніть «Огляд».
MustEnterGroupName=Ви повинні ввести ім'я папки.
GroupNameTooLong=Ім’я папки або шлях до неї перевищують допустиму довжину.
InvalidGroupName=Вказане ім’я папки недопустиме.
BadGroupName=Ім'я папки не може включати наступні символи:%n%n%1
NoProgramGroupCheck2=&Не створювати папку в меню «Пуск»

; *** "Ready to Install" wizard page
WizardReady=Усе готово до встановлення
ReadyLabel1=Програма готова розпочати встановлення [name] на ваш комп’ютер.
ReadyLabel2a=Натисніть «Встановити» для продовження встановлення, або «Назад», якщо ви бажаєте переглянути або змінити налаштування встановлення.
ReadyLabel2b=Натисніть «Встановити» для продовження.
ReadyMemoUserInfo=Дані про користувача:
ReadyMemoDir=Шлях встановлення:
ReadyMemoType=Тип встановлення:
ReadyMemoComponents=Вибрані компоненти:
ReadyMemoGroup=Папка в меню «Пуск»:
ReadyMemoTasks=Додаткові завдання:

; *** "Preparing to Install" wizard page
WizardPreparing=Підготовка до встановлення
PreparingDesc=Програма встановлення готується до встановлення [name] на ваш комп’ютер.
PreviousInstallNotCompleted=Встановлення або видалення попередньої програми не було закінчено. Вам потрібно перезавантажити ваш комп’ютер для завершення минулого встановлення.%n%nПісля перезавантаження відкрийте програму встановлення знову, щоб завершити встановлення [name].
CannotContinue=Встановлення неможливо продовжити. Будь ласка, натисніть «Скасувати» для виходу.
ApplicationsFound=Наступні програми використовують файли, які повинні бути оновлені програмою встановлення. Рекомендується дозволили програмі встановлення автоматично закрити ці програми.
ApplicationsFound2=Наступні програми використовують файли, які повинні бути оновлені програмою встановлення. Рекомендується дозволили програмі встановлення автоматично закрити ці програми. Після завершення встановлення, програма встановлення спробує знову запустити їх.
CloseApplications=&Автоматично закрити програми
DontCloseApplications=&Не закривати програми
ErrorCloseApplications=Setup was unable to automatically close all applications. It is recommended that you close all applications using files that need to be updated by Setup before continuing.

; *** "Installing" wizard page
WizardInstalling=Встановлення
InstallingLabel=Будь ласка, зачекайте, поки [name] встановиться на ваш комп'ютер.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Завершення встановлення [name]
FinishedLabelNoIcons=Встановлення [name] на ваш комп’ютер закінчено.
FinishedLabel=Встановлення [name] на ваш комп’ютер закінчено. Встановлені програми можна відкрити за допомогою створених ярликів.
ClickFinish=Натисніть «Готово» для виходу з програми встановлення.
FinishedRestartLabel=Для завершення встановлення [name] необхідно перезавантажити ваш комп’ютер. Перезавантажити комп’ютер зараз?
FinishedRestartMessage=Для завершення встановлення [name] необхідно перезавантажити ваш комп’ютер.%n%nПерезавантажити комп’ютер зараз?
ShowReadmeCheck=Так, я хочу переглянути файл README
YesRadio=&Так, перезавантажити комп’ютер зараз
NoRadio=&Ні, я перезавантажу комп’ютер пізніше
; used for example as 'Run MyProg.exe'
RunEntryExec=Відкрити %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Переглянути %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=Необхідно вставити наступний диск
SelectDiskLabel2=Будь ласка, вставте диск %1 і натисніть «OK».%n%nЯкщо потрібні файли можуть знаходитися в іншій папці, на відміну від вказаної нижче, введіть правильний шлях або натисніть «Огляд».
PathLabel=&Шлях:
FileNotInDir2=Файл "%1" не знайдений в "%2". Будь ласка, вставте належний диск або вкажіть іншу папку.
SelectDirectoryLabel=Будь ласка, вкажіть шлях до наступного диску.

; *** Installation phase messages
SetupAborted=Встановлення не завершено.%n%nБудь ласка, усуньте проблему і відкрийте програму встановлення знову.
EntryAbortRetryIgnore=Натисніть «Повторити спробу» щоб спробувати ще раз, «Пропустити» щоб пропустити, або «Скасувати» для скасування встановлення.

; *** Installation status messages
StatusClosingApplications=Закриття програм...
StatusCreateDirs=Створення папок...
StatusExtractFiles=Розпакування файлів...
StatusCreateIcons=Створення ярликів...
StatusCreateIniEntries=Створення INI записів...
StatusCreateRegistryEntries=Створення записів реєстру...
StatusRegisterFiles=Реєстрація файлів...
StatusSavingUninstall=Збереження інформації для видалення...
StatusRunProgram=Завершення встановлення...
StatusRestartingApplications=Перезапуск програм...
StatusRollback=Скасування змін...

; *** Misc. errors
ErrorInternal2=Внутрішня помилка: %1
ErrorFunctionFailedNoCode=%1 збій
ErrorFunctionFailed=%1 збій; код %2
ErrorFunctionFailedWithMessage=%1 збій; код %2.%n%3
ErrorExecutingProgram=Неможливо виконати файл:%n%1

; ***  Registry errors
ErrorRegOpenKey=Помилка відкриття ключа реєстру:%n%1\%2
ErrorRegCreateKey=Помилка створення ключа реєстру:%n%1\%2
ErrorRegWriteKey=Помилка запису в ключ реєстру:%n%1\%2

; *** INI errors
ErrorIniEntry=Помилка при створенні запису в INI-файлі "%1".

; *** File copying errors
FileAbortRetryIgnore=Натисніть «Повторити спробу» щоб спробувати ще раз, «Пропустити» щоб пропустити файл (не рекомендується) або «Скасувати» для скасування встановлення.
FileAbortRetryIgnore2=Натисніть «Повторити спробу» щоб спробувати ще раз, «Пропустити» щоб ігнорувати помилку (не рекомендується) або «Скасувати» для скасування встановлення.
SourceIsCorrupted=Вихідний файл пошкоджений
SourceDoesntExist=Вихідний файл "%1" не існує
ExistingFileReadOnly=Існуючий файл помічений як «Лише читання».%n%nНатисніть «Повторити спробу» щоб видалити атрибут «Лише читання», «Пропустити» щоб пропустити файл або «Скасувати» для скасування встановлення.
ErrorReadingExistingDest=Виникла помилка при спробі читання існуючого файлу:
FileExists=Файл вже існує.%n%nПерезаписати його?
ExistingFileNewer=Існуючий файл новіший, чим встановлюваний. Рекомендується зберегти існуючий файл.%n%nВи бажаєте зберегти існуючий файл?
ErrorChangingAttr=Виникла помилка при спробі зміни атрибутів існуючого файлу:
ErrorCreatingTemp=Виникла помилка при спробі створення файлу в папці встановлення:
ErrorReadingSource=Виникла помилка при спробі читання вихідного файлу:
ErrorCopying=Виникла помилка при спробі копіювання файлу:
ErrorReplacingExistingFile=Виникла помилка при спробі заміни існуючого файлу:
ErrorRestartReplace=Помилка RestartReplace:
ErrorRenamingTemp=Виникла помилка при спробі перейменування файлу в папці встановлення:
ErrorRegisterServer=Неможливо зареєструвати DLL/OCX: %1
ErrorRegSvr32Failed=Помилка при виконанні RegSvr32, код повернення %1
ErrorRegisterTypeLib=Неможливо зареєструвати бібліотеку типів: %1

; *** Post-installation errors
ErrorOpeningReadme=Виникла помилка при спробі відкриття файлу README.
ErrorRestartingComputer=Програмі встановлення не вдалося перезавантажити комп'ютер. Будь ласка, виконайте це самостійно.

; *** Uninstaller messages
UninstallNotFound=Файл "%1" не існує, видалення неможливе.
UninstallOpenError=Неможливо відкрити файл "%1". Видалення неможливе
UninstallUnsupportedVer=Файл протоколу для видалення "%1" не розпізнаний даною версією програми видалення. Видалення неможливе
UninstallUnknownEntry=Невідомий запис (%1) в файлі протоколу для видалення
ConfirmUninstall=Ви впевнені, що бажаєте видалити %1 і всі його компоненти?
UninstallOnlyOnWin64=Цю програму можливо видалити лише у середовищі 64-бітної версії Windows.
OnlyAdminCanUninstall=Ця програма може бути видалена лише користувачем з правами адміністратора.
UninstallStatusLabel=Будь ласка, зачекайте, поки %1 видалиться з вашого комп'ютера.
UninstalledAll=%1 успішно видалено з вашого комп'ютера.
UninstalledMost=Видалення %1 закінчено.%n%nДеякі елемент неможливо видалити. Ви можете видалити їх вручну.
UninstalledAndNeedsRestart=Для завершення видалення %1 необхідно перезавантажити ваш комп’ютер.%n%nПерезавантажити комп’ютер зараз?
UninstallDataCorrupted=Файл "%1" пошкоджений. Видалення неможливе

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Видалити загальні файли?
ConfirmDeleteSharedFile2=Система свідчить, що наступний спільний файл більше не використовується іншими програмами. Ви бажаєте видалити цей спільний файл?%n%nЯкщо які-небудь програми все ще використовують цей файл і він видалиться, то ці програми можуть функціонувати неправильно. Якщо ви не впевнені, виберіть «Ні». Залишений файл не нашкодить вашій системі.
SharedFileNameLabel=Ім'я файлу:
SharedFileLocationLabel=Розміщення:
WizardUninstalling=Стан видалення
StatusUninstalling=Видалення %1...

; *** Shutdown block reasons
ShutdownBlockReasonInstallingApp=Встановлення %1.
ShutdownBlockReasonUninstallingApp=Видалення %1.

; The custom messages below aren't used by Setup itself,
; but if you make use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1, версія %2
AdditionalIcons=Додаткові ярлики:
CreateDesktopIcon=Створити ярлики на &Робочому столі
CreateQuickLaunchIcon=Створити ярлики на &Панелі швидкого запуску
ProgramOnTheWeb=Сайт %1 в Інтернеті
UninstallProgram=Видалити %1
LaunchProgram=Відкрити %1
AssocFileExtension=&Асоціювати %1 з розширенням файлу %2
AssocingFileExtension=Асоціювання %1 з розширенням файлу %2...
AutoStartProgramGroupDescription=Автозавантаження:
AutoStartProgram=Автоматично завантужувати %1
AddonHostProgramNotFound=%1 не знайдений у вказаній вами папці%n%nВи все одно бажаєте продовжити?
