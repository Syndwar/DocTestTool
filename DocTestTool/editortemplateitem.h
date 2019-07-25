#ifndef DOC_EDITOR_TEMPLATE_ITEM
#define DOC_EDITOR_TEMPLATE_ITEM

#include <QListWidgetItem>

class EditorTemplateItem : public QListWidgetItem
{
private:
    QStringList m_tags;
public:
    EditorTemplateItem(const QString & text) : QListWidgetItem(text)
    {
    }

    virtual ~EditorTemplateItem()
    {
    }

    const QStringList & tags() const
    {
        return m_tags;
    }

    void setTags(const QStringList & tags)
    {
        m_tags = tags;
    }
};

#endif // DOC_EDITOR_TEMPLATE_ITEM