/* global IApi, CosmoScout */

/**
 * Notifications
 */
class NotificationApi extends IApi {
    /**
     * @inheritDoc
     */
    name = 'notifications';

    /**
     * @type {HTMLElement}
     * @private
     */
    _container;

    /**
     * Set the container in which to place the notifications
     *
     * @param container {string}
     */
    init(container = 'notifications') {
      this._container = document.getElementById(container);
    }

    /**
     * Adds a notification into the initialized notification container
     *
     * @param title {string} Title
     * @param content {string} Content
     * @param icon {string} Materialize Icon Name
     * @param flyTo {string} Optional flyto name which gets passed to 'fly_to'. Activated on click
     */
    printNotification(title, content, icon, flyTo) {
      if (this._container.children.length > 4) {
        const no = this._container.lastElementChild;

        clearTimeout(no.timer);

        this._container.removeChild(no);
      }

      let notification;
      try {
        notification = NotificationApi.makeNotification(title, content, icon);
      } catch (e) {
        return;
      }

      this._container.prepend(notification);

      if (flyTo) {
        notification.classList.add('clickable');
        notification.addEventListener('click', () => {
          CosmoScout.flyto.flyTo(flyTo);
        });
      }

      notification.timer = setTimeout(() => {
        notification.classList.add('fadeout');
        this._container.removeChild(notification);
      }, 8000);

      setTimeout(() => {
        notification.classList.add('show');
      }, 60);
    }

    /**
     * Creates the actual HTML Notification
     *
     * @param title {string}
     * @param content {string}
     * @param icon {string}
     * @return {HTMLElement}
     * @private
     */
    static makeNotification(title, content, icon = '') {
      const notification = CosmoScout.loadTemplateContent('notification');

      if (notification === false) {
        throw new Error();
      }

      notification.innerHTML = notification.innerHTML
        .replace('%TITLE%', title)
        .replace('%CONTENT%', content)
        .replace('%ICON%', icon)
        .trim();

      return notification;
    }
}
