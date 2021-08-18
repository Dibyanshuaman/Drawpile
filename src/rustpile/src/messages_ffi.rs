// Generated with protogen-builder.py

use dpcore::paint::UserID;
use dpcore::protocol::message::*;
use dpcore::protocol::MessageWriter;
use std::slice;

#[no_mangle]
pub extern "C" fn messagewriter_new() -> *mut MessageWriter {
    Box::into_raw(Box::new(MessageWriter::new()))
}

#[no_mangle]
pub extern "C" fn messagewriter_free(mw: *mut MessageWriter) {
    if !mw.is_null() {
        unsafe { Box::from_raw(mw) };
    }
}

#[no_mangle]
pub extern "C" fn messagewriter_content(mw: &MessageWriter, len: &mut usize) -> *const u8 {
    mw.as_ptr(len)
}

#[no_mangle]
pub extern "C" fn write_servercommand(
    writer: &mut MessageWriter,
    ctx: UserID,
    msg: *const u8,
    msg_len: usize,
) {
    ControlMessage::ServerCommand(
        ctx,
        String::from_utf8_lossy(unsafe { slice::from_raw_parts(msg, msg_len) }).to_string(),
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_sessionowner(
    writer: &mut MessageWriter,
    ctx: UserID,
    users: *const u8,
    users_len: usize,
) {
    ServerMetaMessage::SessionOwner(
        ctx,
        unsafe { slice::from_raw_parts(users, users_len) }.into(),
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_chat(
    writer: &mut MessageWriter,
    ctx: UserID,
    flags: u8,
    message: *const u8,
    message_len: usize,
) {
    ServerMetaMessage::Chat(
        ctx,
        ChatMessage {
            flags: flags,
            message: String::from_utf8_lossy(unsafe {
                slice::from_raw_parts(message, message_len)
            })
            .to_string(),
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_trusted(
    writer: &mut MessageWriter,
    ctx: UserID,
    users: *const u8,
    users_len: usize,
) {
    ServerMetaMessage::TrustedUsers(
        ctx,
        unsafe { slice::from_raw_parts(users, users_len) }.into(),
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_privatechat(
    writer: &mut MessageWriter,
    ctx: UserID,
    target: u8,
    flags: u8,
    message: *const u8,
    message_len: usize,
) {
    ServerMetaMessage::PrivateChat(
        ctx,
        PrivateChatMessage {
            target: target,
            flags: flags,
            message: String::from_utf8_lossy(unsafe {
                slice::from_raw_parts(message, message_len)
            })
            .to_string(),
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_lasertrail(
    writer: &mut MessageWriter,
    ctx: UserID,
    color: u32,
    persistence: u8,
) {
    ClientMetaMessage::LaserTrail(
        ctx,
        LaserTrailMessage {
            color: color,
            persistence: persistence,
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_movepointer(writer: &mut MessageWriter, ctx: UserID, x: i32, y: i32) {
    ClientMetaMessage::MovePointer(ctx, MovePointerMessage { x: x, y: y }).write(writer);
}

#[no_mangle]
pub extern "C" fn write_marker(
    writer: &mut MessageWriter,
    ctx: UserID,
    text: *const u8,
    text_len: usize,
) {
    ClientMetaMessage::Marker(
        ctx,
        String::from_utf8_lossy(unsafe { slice::from_raw_parts(text, text_len) }).to_string(),
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_useracl(
    writer: &mut MessageWriter,
    ctx: UserID,
    users: *const u8,
    users_len: usize,
) {
    ClientMetaMessage::UserACL(
        ctx,
        unsafe { slice::from_raw_parts(users, users_len) }.into(),
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_layeracl(
    writer: &mut MessageWriter,
    ctx: UserID,
    id: u16,
    flags: u8,
    exclusive: *const u8,
    exclusive_len: usize,
) {
    ClientMetaMessage::LayerACL(
        ctx,
        LayerACLMessage {
            id: id,
            flags: flags,
            exclusive: unsafe { slice::from_raw_parts(exclusive, exclusive_len) }.into(),
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_featureaccess(
    writer: &mut MessageWriter,
    ctx: UserID,
    feature_tiers: *const u8,
    feature_tiers_len: usize,
) {
    ClientMetaMessage::FeatureAccessLevels(
        ctx,
        unsafe { slice::from_raw_parts(feature_tiers, feature_tiers_len) }.into(),
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_defaultlayer(writer: &mut MessageWriter, ctx: UserID, id: u16) {
    ClientMetaMessage::DefaultLayer(ctx, id).write(writer);
}

#[no_mangle]
pub extern "C" fn write_undopoint(writer: &mut MessageWriter, ctx: UserID) {
    CommandMessage::UndoPoint(ctx).write(writer);
}

#[no_mangle]
pub extern "C" fn write_resize(
    writer: &mut MessageWriter,
    ctx: UserID,
    top: i32,
    right: i32,
    bottom: i32,
    left: i32,
) {
    CommandMessage::CanvasResize(
        ctx,
        CanvasResizeMessage {
            top: top,
            right: right,
            bottom: bottom,
            left: left,
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_newlayer(
    writer: &mut MessageWriter,
    ctx: UserID,
    id: u16,
    source: u16,
    fill: u32,
    flags: u8,
    name: *const u8,
    name_len: usize,
) {
    CommandMessage::LayerCreate(
        ctx,
        LayerCreateMessage {
            id: id,
            source: source,
            fill: fill,
            flags: flags,
            name: String::from_utf8_lossy(unsafe { slice::from_raw_parts(name, name_len) })
                .to_string(),
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_layerattr(
    writer: &mut MessageWriter,
    ctx: UserID,
    id: u16,
    sublayer: u8,
    flags: u8,
    opacity: u8,
    blend: u8,
) {
    CommandMessage::LayerAttributes(
        ctx,
        LayerAttributesMessage {
            id: id,
            sublayer: sublayer,
            flags: flags,
            opacity: opacity,
            blend: blend,
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_retitlelayer(
    writer: &mut MessageWriter,
    ctx: UserID,
    id: u16,
    title: *const u8,
    title_len: usize,
) {
    CommandMessage::LayerRetitle(
        ctx,
        LayerRetitleMessage {
            id: id,
            title: String::from_utf8_lossy(unsafe { slice::from_raw_parts(title, title_len) })
                .to_string(),
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_layerorder(
    writer: &mut MessageWriter,
    ctx: UserID,
    layers: *const u16,
    layers_len: usize,
) {
    CommandMessage::LayerOrder(
        ctx,
        unsafe { slice::from_raw_parts(layers, layers_len) }.into(),
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_deletelayer(writer: &mut MessageWriter, ctx: UserID, id: u16, merge: bool) {
    CommandMessage::LayerDelete(
        ctx,
        LayerDeleteMessage {
            id: id,
            merge: merge,
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_layervisibility(
    writer: &mut MessageWriter,
    ctx: UserID,
    id: u16,
    visible: bool,
) {
    CommandMessage::LayerVisibility(
        ctx,
        LayerVisibilityMessage {
            id: id,
            visible: visible,
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_putimage(
    writer: &mut MessageWriter,
    ctx: UserID,
    layer: u16,
    mode: u8,
    x: u32,
    y: u32,
    w: u32,
    h: u32,
    image: *const u8,
    image_len: usize,
) {
    CommandMessage::PutImage(
        ctx,
        PutImageMessage {
            layer: layer,
            mode: mode,
            x: x,
            y: y,
            w: w,
            h: h,
            image: unsafe { slice::from_raw_parts(image, image_len) }.into(),
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_fillrect(
    writer: &mut MessageWriter,
    ctx: UserID,
    layer: u16,
    mode: u8,
    x: u32,
    y: u32,
    w: u32,
    h: u32,
    color: u32,
) {
    CommandMessage::FillRect(
        ctx,
        FillRectMessage {
            layer: layer,
            mode: mode,
            x: x,
            y: y,
            w: w,
            h: h,
            color: color,
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_penup(writer: &mut MessageWriter, ctx: UserID) {
    CommandMessage::PenUp(ctx).write(writer);
}

#[no_mangle]
pub extern "C" fn write_newannotation(
    writer: &mut MessageWriter,
    ctx: UserID,
    id: u16,
    x: i32,
    y: i32,
    w: u16,
    h: u16,
) {
    CommandMessage::AnnotationCreate(
        ctx,
        AnnotationCreateMessage {
            id: id,
            x: x,
            y: y,
            w: w,
            h: h,
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_reshapeannotation(
    writer: &mut MessageWriter,
    ctx: UserID,
    id: u16,
    x: i32,
    y: i32,
    w: u16,
    h: u16,
) {
    CommandMessage::AnnotationReshape(
        ctx,
        AnnotationReshapeMessage {
            id: id,
            x: x,
            y: y,
            w: w,
            h: h,
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_editannotation(
    writer: &mut MessageWriter,
    ctx: UserID,
    id: u16,
    bg: u32,
    flags: u8,
    border: u8,
    text: *const u8,
    text_len: usize,
) {
    CommandMessage::AnnotationEdit(
        ctx,
        AnnotationEditMessage {
            id: id,
            bg: bg,
            flags: flags,
            border: border,
            text: String::from_utf8_lossy(unsafe { slice::from_raw_parts(text, text_len) })
                .to_string(),
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_deleteannotation(writer: &mut MessageWriter, ctx: UserID, id: u16) {
    CommandMessage::AnnotationDelete(ctx, id).write(writer);
}

#[no_mangle]
pub extern "C" fn write_puttile(
    writer: &mut MessageWriter,
    ctx: UserID,
    layer: u16,
    sublayer: u8,
    col: u16,
    row: u16,
    repeat: u16,
    image: *const u8,
    image_len: usize,
) {
    CommandMessage::PutTile(
        ctx,
        PutTileMessage {
            layer: layer,
            sublayer: sublayer,
            col: col,
            row: row,
            repeat: repeat,
            image: unsafe { slice::from_raw_parts(image, image_len) }.into(),
        },
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_background(
    writer: &mut MessageWriter,
    ctx: UserID,
    image: *const u8,
    image_len: usize,
) {
    CommandMessage::CanvasBackground(
        ctx,
        unsafe { slice::from_raw_parts(image, image_len) }.into(),
    )
    .write(writer);
}

#[no_mangle]
pub extern "C" fn write_undo(
    writer: &mut MessageWriter,
    ctx: UserID,
    override_user: u8,
    redo: bool,
) {
    CommandMessage::Undo(
        ctx,
        UndoMessage {
            override_user: override_user,
            redo: redo,
        },
    )
    .write(writer);
}