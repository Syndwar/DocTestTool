#include "authdialog.h"
#include "ui_authdialog.h"
#include "share/debug.h"
#include "settings/settingsmanager.h"

AuthDialog::AuthDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthDialog)
{
    init();
}

AuthDialog::~AuthDialog()
{  
    delete ui;
}

void AuthDialog::init(void)
{
    SettingsManager settingsManager;

    ui->setupUi(this);

    auth.reset(new Auth);

    //ui->webView->setUrl(auth->getOAuth2CodeUrl(settingsManager.scope(), settingsManager.redirectUri(), settingsManager.clientId(), false, true).query());

    QUrlQuery urlQuery(auth->getOAuth2CodeUrl(settingsManager.scope(), settingsManager.redirectUri(), settingsManager.clientId(), false, true));
    QString urlStr("https://accounts.google.com/o/oauth2/auth?" + urlQuery.toString());
    ui->webView->load(urlStr);
//    ensureVisible();

    connect(ui->webView, SIGNAL(urlChanged(const QUrl&)), this, SLOT(slotUrlChanged(const QUrl&)));
    connect(auth.data(), SIGNAL(signalAuthResponse(const QString&, const QString&)), this, SLOT(slotAuthResponse(const QString&, const QString&)));
}

void AuthDialog::slotUrlChanged(const QUrl &url)
{
    SettingsManager settingsManager;
    QString code(auth->getOAuth2Code(QUrlQuery(url)));

    if(!code.isEmpty())
    {
       auth->getTokens(code, settingsManager.clientId(), settingsManager.clientSecret(), settingsManager.redirectUri());
    }
}

void AuthDialog::slotAuthResponse(const QString &accessToken, const QString &refreshToken)
{
    if (!accessToken.isEmpty())
        emit signalTokens(accessToken, refreshToken);

    QDialog::accept();
}
